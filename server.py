#!/usr/bin/env python3
"""
SrikanthMart — Local Interactive Server Runner
Provides an instant local environment on Windows/Linux matching the native Drogon C++20 API.
Owner: Srikanth
"""

import http.server
import socketserver
import json
import sqlite3
import os
import re
import mimetypes
from urllib.parse import urlparse, parse_qs

PORT = int(os.environ.get("PORT", 8080))
DB_PATH = "srikanthmart.db"
MIGRATIONS_DIR = os.path.join("db", "migrations")
SEED_FILE = os.path.join("db", "seed.sql")
PUBLIC_DIR = "public"

def init_database():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    cursor.execute("""
    CREATE TABLE IF NOT EXISTS schema_migrations (
        version VARCHAR(64) PRIMARY KEY,
        applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
    """)

    # Apply migrations in order
    if os.path.exists(MIGRATIONS_DIR):
        files = sorted([f for f in os.listdir(MIGRATIONS_DIR) if f.endswith(".sql")])
        for f in files:
            cursor.execute("SELECT 1 FROM schema_migrations WHERE version = ?", (f,))
            if not cursor.fetchone():
                print(f"[Migration] Applying {f}...")
                with open(os.path.join(MIGRATIONS_DIR, f), "r", encoding="utf-8") as sf:
                    cursor.executescript(sf.read())
                cursor.execute("INSERT INTO schema_migrations (version) VALUES (?)", (f,))
                conn.commit()

    # Apply seed data
    cursor.execute("SELECT COUNT(*) FROM users;")
    count = cursor.fetchone()[0]
    if count == 0 and os.path.exists(SEED_FILE):
        print(f"[Seed] Populating initial demo data from {SEED_FILE}...")
        with open(SEED_FILE, "r", encoding="utf-8") as sf:
            cursor.executescript(sf.read())
        conn.commit()

    conn.close()

def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

class SrikanthMartHandler(http.server.BaseHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Request-Id")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

    def send_json(self, status_code, data, is_error=False, code=None, message=None):
        self.send_response(status_code)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        envelope = {
            "success": not is_error,
            "data": data if not is_error else None,
            "error": {"code": code, "message": message, "field": None} if is_error else None
        }
        self.wfile.write(json.dumps(envelope).encode("utf-8"))

    def get_auth_user_id(self):
        auth = self.headers.get("Authorization", "")
        if auth.startswith("Bearer user_"):
            try:
                return int(auth.split("Bearer user_")[1])
            except ValueError:
                pass
        return 3 # Default to buyer

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path
        query = parse_qs(parsed.query)

        if path == "/api/v1/health":
            self.send_json(200, {"status": "UP", "db": "UP", "app": "SrikanthMart", "version": "1.1.0"})
            return

        conn = get_db()
        cursor = conn.cursor()

        # Products Listing
        if path == "/api/v1/products":
            q = query.get("q", [""])[0]
            cat = query.get("category", [""])[0]

            sql = "SELECT id, seller_id, name, description, price_cents, stock_qty, category, image_url, created_at FROM products WHERE 1=1"
            params = []
            if q:
                sql += " AND (name LIKE ? OR description LIKE ?)"
                params.extend([f"%{q}%", f"%{q}%"])
            if cat:
                sql += " AND category = ?"
                params.append(cat)
            sql += " ORDER BY id DESC"

            cursor.execute(sql, params)
            rows = [dict(r) for r in cursor.fetchall()]
            for r in rows:
                r["formatted_price"] = f"INR {r['price_cents']/100:.2f}"
            self.send_json(200, rows)
            conn.close()
            return

        # Single Product
        match_prod = re.match(r"^/api/v1/products/(\d+)$", path)
        if match_prod:
            pid = int(match_prod.group(1))
            cursor.execute("SELECT * FROM products WHERE id = ?", (pid,))
            row = cursor.fetchone()
            if not row:
                self.send_json(404, None, is_error=True, code="NOT_FOUND", message="Product not found")
            else:
                p = dict(row)
                p["formatted_price"] = f"INR {p['price_cents']/100:.2f}"
                self.send_json(200, p)
            conn.close()
            return

        # Product Reviews
        match_reviews = re.match(r"^/api/v1/products/(\d+)/reviews$", path)
        if match_reviews:
            pid = int(match_reviews.group(1))
            cursor.execute("""
                SELECT r.id, r.product_id, r.user_id, r.rating, r.comment, r.created_at, u.name as reviewer_name
                FROM reviews r JOIN users u ON r.user_id = u.id
                WHERE r.product_id = ? ORDER BY r.id DESC
            """, (pid,))
            rows = [dict(r) for r in cursor.fetchall()]
            self.send_json(200, rows)
            conn.close()
            return

        # Cart
        if path == "/api/v1/cart":
            uid = self.get_auth_user_id()
            cursor.execute("""
                SELECT c.id, c.user_id, c.product_id, c.quantity, p.name as product_name,
                       p.price_cents as unit_price_cents, p.image_url as product_image_url
                FROM cart_items c JOIN products p ON c.product_id = p.id
                WHERE c.user_id = ?
            """, (uid,))
            items = [dict(r) for r in cursor.fetchall()]
            total_cents = 0
            total_count = 0
            for it in items:
                line_total = it["unit_price_cents"] * it["quantity"]
                it["line_total_cents"] = line_total
                it["formatted_unit_price"] = f"INR {it['unit_price_cents']/100:.2f}"
                it["formatted_line_total"] = f"INR {line_total/100:.2f}"
                total_cents += line_total
                total_count += it["quantity"]

            summary = {
                "items": items,
                "total_items_count": total_count,
                "total_amount_cents": total_cents,
                "formatted_total": f"INR {total_cents/100:.2f}"
            }
            self.send_json(200, summary)
            conn.close()
            return

        # Buyer Orders
        if path == "/api/v1/orders/buyer":
            uid = self.get_auth_user_id()
            cursor.execute("SELECT * FROM orders WHERE buyer_id = ? ORDER BY id DESC", (uid,))
            orders = [dict(r) for r in cursor.fetchall()]
            for o in orders:
                o["formatted_total"] = f"INR {o['total_amount_cents']/100:.2f}"
                cursor.execute("""
                    SELECT oi.*, p.name as product_name FROM order_items oi
                    JOIN products p ON oi.product_id = p.id WHERE oi.order_id = ?
                """, (o["id"],))
                o["items"] = [dict(ir) for ir in cursor.fetchall()]
                for it in o["items"]:
                    it["formatted_unit_price"] = f"INR {it['unit_price_cents']/100:.2f}"
            self.send_json(200, orders)
            conn.close()
            return

        # Seller Products
        if path == "/api/v1/products/seller/mine":
            uid = self.get_auth_user_id()
            cursor.execute("SELECT * FROM products WHERE seller_id = ? ORDER BY id DESC", (uid,))
            products = [dict(r) for r in cursor.fetchall()]
            for p in products:
                p["formatted_price"] = f"INR {p['price_cents']/100:.2f}"
            self.send_json(200, products)
            conn.close()
            return

        # Seller Incoming Orders
        if path == "/api/v1/orders/seller":
            uid = self.get_auth_user_id()
            cursor.execute("""
                SELECT DISTINCT o.* FROM orders o
                JOIN order_items oi ON o.id = oi.order_id
                JOIN products p ON oi.product_id = p.id
                WHERE p.seller_id = ? ORDER BY o.id DESC
            """, (uid,))
            orders = [dict(r) for r in cursor.fetchall()]
            for o in orders:
                o["formatted_total"] = f"INR {o['total_amount_cents']/100:.2f}"
            self.send_json(200, orders)
            conn.close()
            return

        # Seller Dashboard
        if path == "/api/v1/orders/seller/dashboard":
            uid = self.get_auth_user_id()
            cursor.execute("""
                SELECT COUNT(DISTINCT o.id), COALESCE(SUM(oi.quantity * oi.unit_price_cents), 0)
                FROM orders o JOIN order_items oi ON o.id = oi.order_id
                JOIN products p ON oi.product_id = p.id
                WHERE p.seller_id = ? AND o.status != 'CANCELLED'
            """, (uid,))
            cnt, rev = cursor.fetchone()
            cursor.execute("SELECT COUNT(*) FROM products WHERE seller_id = ?", (uid,))
            prod_cnt = cursor.fetchone()[0]

            self.send_json(200, {
                "total_orders_count": cnt,
                "total_revenue_cents": rev,
                "formatted_revenue": f"INR {rev/100:.2f}",
                "total_products_listed": prod_cnt
            })
            conn.close()
            return

        # Admin Users
        if path == "/api/v1/admin/users":
            cursor.execute("SELECT id, name, email, role, created_at FROM users ORDER BY id ASC")
            users = [dict(r) for r in cursor.fetchall()]
            self.send_json(200, users)
            conn.close()
            return

        # Admin Orders
        if path == "/api/v1/admin/orders":
            cursor.execute("SELECT * FROM orders ORDER BY id DESC")
            orders = [dict(r) for r in cursor.fetchall()]
            for o in orders:
                o["formatted_total"] = f"INR {o['total_amount_cents']/100:.2f}"
            self.send_json(200, orders)
            conn.close()
            return

        conn.close()

        # Static File Serving
        self.serve_static(path)

    def do_POST(self):
        path = urlparse(self.path).path
        content_length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_length).decode("utf-8") if content_length > 0 else "{}"
        try:
            payload = json.loads(body)
        except json.JSONDecodeError:
            self.send_json(400, None, is_error=True, code="VALIDATION_ERROR", message="Invalid JSON")
            return

        conn = get_db()
        cursor = conn.cursor()

        # AI Chatbot
        if path == "/api/v1/chat":
            msg = payload.get("message", "").lower()
            reply = "Hello! I am SrikanthMart AI Assistant. I can assist you with tracking orders, discovering products, checking return policies, or selling on our marketplace!"
            if "status" in msg or "track" in msg:
                reply = "Live order status workflow follows: Pending -> Confirmed -> Shipped -> Delivered. Track it anytime under 'My Orders'!"
            elif "return" in msg or "refund" in msg:
                reply = "SrikanthMart offers a 14-day hassle-free replacement or refund guarantee on all products."
            elif "shipping" in msg or "delivery" in msg:
                reply = "Standard express delivery is free on all orders and arrives in 2 to 4 business days."
            elif "laptop" in msg:
                reply = "The UltraBook Pro 15 with 16GB RAM and 512GB SSD is available for INR 45,000 in the Electronics section!"
            elif "mobile" in msg:
                reply = "The Nexus 5G Smartphone is available for INR 18,000 with 120Hz AMOLED display!"
            elif "owner" in msg or "srikanth" in msg:
                reply = "SrikanthMart is owned and architected by Srikanth."

            self.send_json(200, {"reply": reply})
            conn.close()
            return

        # Add to Cart
        if path == "/api/v1/cart/items":
            uid = self.get_auth_user_id()
            pid = payload.get("product_id")
            qty = payload.get("quantity", 1)

            cursor.execute("SELECT id, quantity FROM cart_items WHERE user_id = ? AND product_id = ?", (uid, pid))
            row = cursor.fetchone()
            if row:
                cursor.execute("UPDATE cart_items SET quantity = quantity + ? WHERE id = ?", (qty, row["id"]))
            else:
                cursor.execute("INSERT INTO cart_items (user_id, product_id, quantity) VALUES (?, ?, ?)", (uid, pid, qty))
            conn.commit()
            conn.close()
            self.do_GET_cart_summary()
            return

        # Checkout Order
        if path == "/api/v1/orders/checkout":
            uid = self.get_auth_user_id()
            cursor.execute("""
                SELECT c.product_id, c.quantity, p.price_cents FROM cart_items c
                JOIN products p ON c.product_id = p.id WHERE c.user_id = ?
            """, (uid,))
            cart_rows = cursor.fetchall()
            if not cart_rows:
                self.send_json(400, None, is_error=True, code="VALIDATION_ERROR", message="Cannot checkout with empty cart")
                conn.close()
                return

            total_cents = sum(r["quantity"] * r["price_cents"] for r in cart_rows)
            cursor.execute("INSERT INTO orders (buyer_id, status, total_amount_cents) VALUES (?, 'PENDING', ?)", (uid, total_cents))
            order_id = cursor.lastrowid

            for r in cart_rows:
                cursor.execute("""
                    INSERT INTO order_items (order_id, product_id, quantity, unit_price_cents)
                    VALUES (?, ?, ?, ?)
                """, (order_id, r["product_id"], r["quantity"], r["price_cents"]))
                cursor.execute("UPDATE products SET stock_qty = stock_qty - ? WHERE id = ?", (r["quantity"], r["product_id"]))

            cursor.execute("DELETE FROM cart_items WHERE user_id = ?", (uid,))
            conn.commit()
            conn.close()

            self.send_json(201, {
                "id": order_id,
                "buyer_id": uid,
                "status": "PENDING",
                "total_amount_cents": total_cents,
                "formatted_total": f"INR {total_cents/100:.2f}"
            })
            return

        # Create Product Listing (Seller)
        if path == "/api/v1/products":
            uid = self.get_auth_user_id()
            cursor.execute("""
                INSERT INTO products (seller_id, name, description, price_cents, stock_qty, category, image_url)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            """, (uid, payload.get("name"), payload.get("description"), payload.get("price_cents"),
                  payload.get("stock_qty"), payload.get("category"), payload.get("image_url")))
            conn.commit()
            pid = cursor.lastrowid
            conn.close()
            self.send_json(201, {"id": pid, "message": "Product listed successfully"})
            return

        # Add Product Review
        match_rev = re.match(r"^/api/v1/products/(\d+)/reviews$", path)
        if match_rev:
            pid = int(match_rev.group(1))
            uid = self.get_auth_user_id()
            cursor.execute("""
                INSERT INTO reviews (product_id, user_id, rating, comment)
                VALUES (?, ?, ?, ?)
            """, (pid, uid, payload.get("rating", 5), payload.get("comment", "")))
            conn.commit()
            conn.close()
            self.send_json(201, {"message": "Review recorded successfully"})
            return

        conn.close()
        self.send_json(404, None, is_error=True, code="NOT_FOUND", message="Endpoint not found")

    def do_PUT(self):
        path = urlparse(self.path).path
        content_length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_length).decode("utf-8") if content_length > 0 else "{}"
        try:
            payload = json.loads(body)
        except json.JSONDecodeError:
            self.send_json(400, None, is_error=True, code="VALIDATION_ERROR", message="Invalid JSON")
            return

        conn = get_db()
        cursor = conn.cursor()

        # Update Cart Item Qty
        match_cart = re.match(r"^/api/v1/cart/items/(\d+)$", path)
        if match_cart:
            pid = int(match_cart.group(1))
            uid = self.get_auth_user_id()
            qty = payload.get("quantity", 1)
            if qty <= 0:
                cursor.execute("DELETE FROM cart_items WHERE user_id = ? AND product_id = ?", (uid, pid))
            else:
                cursor.execute("UPDATE cart_items SET quantity = ? WHERE user_id = ? AND product_id = ?", (qty, uid, pid))
            conn.commit()
            conn.close()
            self.do_GET_cart_summary()
            return

        # Update Order Status
        match_order = re.match(r"^/api/v1/orders/(\d+)/status$", path)
        if match_order:
            oid = int(match_order.group(1))
            status = payload.get("status", "CONFIRMED")
            cursor.execute("UPDATE orders SET status = ? WHERE id = ?", (status, oid))
            conn.commit()
            conn.close()
            self.send_json(200, {"order_id": oid, "status": status})
            return

        # Update Product
        match_prod = re.match(r"^/api/v1/products/(\d+)$", path)
        if match_prod:
            pid = int(match_prod.group(1))
            cursor.execute("""
                UPDATE products SET name = ?, description = ?, price_cents = ?, stock_qty = ?, category = ?, image_url = ?
                WHERE id = ?
            """, (payload.get("name"), payload.get("description"), payload.get("price_cents"),
                  payload.get("stock_qty"), payload.get("category"), payload.get("image_url"), pid))
            conn.commit()
            conn.close()
            self.send_json(200, {"id": pid, "message": "Product updated"})
            return

        conn.close()

    def do_DELETE(self):
        path = urlparse(self.path).path
        conn = get_db()
        cursor = conn.cursor()

        # Remove from cart
        match_cart = re.match(r"^/api/v1/cart/items/(\d+)$", path)
        if match_cart:
            pid = int(match_cart.group(1))
            uid = self.get_auth_user_id()
            cursor.execute("DELETE FROM cart_items WHERE user_id = ? AND product_id = ?", (uid, pid))
            conn.commit()
            conn.close()
            self.do_GET_cart_summary()
            return

        # Delete product
        match_prod = re.match(r"^/api/v1/products/(\d+)$", path)
        if match_prod:
            pid = int(match_prod.group(1))
            cursor.execute("DELETE FROM products WHERE id = ?", (pid,))
            conn.commit()
            conn.close()
            self.send_json(200, {"message": "Product deleted"})
            return

        # Admin delete product
        match_admin_prod = re.match(r"^/api/v1/admin/products/(\d+)$", path)
        if match_admin_prod:
            pid = int(match_admin_prod.group(1))
            cursor.execute("DELETE FROM products WHERE id = ?", (pid,))
            conn.commit()
            conn.close()
            self.send_json(200, {"message": "Product removed by admin"})
            return

        conn.close()

    def do_GET_cart_summary(self):
        conn = get_db()
        cursor = conn.cursor()
        uid = self.get_auth_user_id()
        cursor.execute("""
            SELECT c.id, c.user_id, c.product_id, c.quantity, p.name as product_name,
                   p.price_cents as unit_price_cents, p.image_url as product_image_url
            FROM cart_items c JOIN products p ON c.product_id = p.id
            WHERE c.user_id = ?
        """, (uid,))
        items = [dict(r) for r in cursor.fetchall()]
        total_cents = 0
        total_count = 0
        for it in items:
            line_total = it["unit_price_cents"] * it["quantity"]
            it["line_total_cents"] = line_total
            it["formatted_unit_price"] = f"INR {it['unit_price_cents']/100:.2f}"
            it["formatted_line_total"] = f"INR {line_total/100:.2f}"
            total_cents += line_total
            total_count += it["quantity"]

        summary = {
            "items": items,
            "total_items_count": total_count,
            "total_amount_cents": total_cents,
            "formatted_total": f"INR {total_cents/100:.2f}"
        }
        conn.close()
        self.send_json(200, summary)

    def serve_static(self, req_path):
        if req_path == "/" or req_path == "":
            req_path = "/index.html"
        safe_path = os.path.normpath(os.path.join(PUBLIC_DIR, req_path.lstrip("/")))
        if os.path.exists(safe_path) and os.path.isfile(safe_path):
            mime_type, _ = mimetypes.guess_type(safe_path)
            self.send_response(200)
            self.send_header("Content-Type", mime_type or "application/octet-stream")
            self.end_headers()
            with open(safe_path, "rb") as f:
                self.wfile.write(f.read())
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"File not found")

def main():
    init_database()
    print("=" * 60)
    print("  SrikanthMart Platform Server")
    print("  Owner & Architect: Srikanth")
    print(f"  Listening on http://localhost:{PORT}")
    print("=" * 60)
    with socketserver.TCPServer(("", PORT), SrikanthMartHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server gracefully...")

if __name__ == "__main__":
    main()
