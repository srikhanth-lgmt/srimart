-- SudharshanMart Seed Data (PostgreSQL)
-- Optional marketplace sample data only. Administrator accounts are created with --seed-admin.

INSERT INTO users (id, name, email, password_hash, role, created_at) VALUES
(2, 'TechNova Seller', 'seller@sudharshanmart.com', '!DISABLED_SAMPLE_ACCOUNT!', 'SELLER', CURRENT_TIMESTAMP),
(3, 'Akash Buyer', 'buyer@sudharshanmart.com', '!DISABLED_SAMPLE_ACCOUNT!', 'BUYER', CURRENT_TIMESTAMP),
(4, 'Priya Electronics', 'priya@sudharshanmart.com', '!DISABLED_SAMPLE_ACCOUNT!', 'SELLER', CURRENT_TIMESTAMP)
ON CONFLICT DO NOTHING;

-- Sample Marketplace Products (Prices in minor units / cents)
INSERT INTO products (id, seller_id, name, description, price_cents, stock_qty, category, image_url, created_at) VALUES
(1, 2, 'UltraBook Pro 15', 'High-performance laptop with 16GB RAM, 512GB NVMe SSD, and 12-core processor for developers and creators.', 4500000, 25, 'Electronics', 'https://images.unsplash.com/photo-1496181133206-80ce9b88a853?w=600&auto=format&fit=crop&q=80', CURRENT_TIMESTAMP),
(2, 2, 'Nexus 5G Smartphone', 'Flagship 5G smartphone with 120Hz AMOLED display, 64MP triple camera, and 5000mAh battery.', 1800000, 40, 'Electronics', 'https://images.unsplash.com/photo-1511707171634-5f897ff02aa9?w=600&auto=format&fit=crop&q=80', CURRENT_TIMESTAMP),
(3, 2, 'AcousticANC Wireless Headphones', 'Studio-quality active noise-cancelling over-ear headphones with 40-hour battery life.', 450000, 50, 'Audio', 'https://images.unsplash.com/photo-1505740420928-5e560c06d30e?w=600&auto=format&fit=crop&q=80', CURRENT_TIMESTAMP),
(4, 4, 'Ergonomic Mechanical Keyboard', 'RGB backlit mechanical keyboard with hot-swappable switches and custom PBT keycaps.', 650000, 30, 'Accessories', 'https://images.unsplash.com/photo-1587829741301-dc798b83add3?w=600&auto=format&fit=crop&q=80', CURRENT_TIMESTAMP),
(5, 4, 'Pro Gamer Ultra Precision Mouse', 'Lightweight wireless gaming mouse with 26000 DPI sensor and optical switches.', 299900, 60, 'Accessories', 'https://images.unsplash.com/photo-1527864550417-7fd91fc51a46?w=600&auto=format&fit=crop&q=80', CURRENT_TIMESTAMP),
(6, 4, 'Sudharshan Travel Tech Backpack', 'Water-resistant laptop backpack with dedicated USB charging port and anti-theft compartments.', 349900, 35, 'Bags', 'https://images.unsplash.com/photo-1553062407-98eeb64c6a62?w=600&auto=format&fit=crop&q=80', CURRENT_TIMESTAMP)
ON CONFLICT DO NOTHING;

-- Sample Reviews
INSERT INTO reviews (id, product_id, user_id, rating, comment, created_at) VALUES
(1, 1, 3, 5, 'Exceptional build quality! Compiles C++ projects in seconds. Highly recommended by SudharshanMart!', CURRENT_TIMESTAMP),
(2, 2, 3, 4, 'Smooth display and great battery performance. Very swift delivery from seller.', CURRENT_TIMESTAMP),
(3, 3, 3, 5, 'Superb noise cancellation. Comfortable for long hours of coding.', CURRENT_TIMESTAMP)
ON CONFLICT DO NOTHING;

-- Sample Order & Order Items
INSERT INTO orders (id, buyer_id, status, total_amount_cents, created_at) VALUES
(1, 3, 'DELIVERED', 4500000, CURRENT_TIMESTAMP),
(2, 3, 'CONFIRMED', 450000, CURRENT_TIMESTAMP)
ON CONFLICT DO NOTHING;

INSERT INTO order_items (id, order_id, product_id, quantity, unit_price_cents, created_at) VALUES
(1, 1, 1, 1, 4500000, CURRENT_TIMESTAMP),
(2, 2, 3, 1, 450000, CURRENT_TIMESTAMP)
ON CONFLICT DO NOTHING;

-- Realign SERIAL sequences with the explicit IDs inserted above so that future
-- auto-generated identifiers do not collide with the seed rows.
SELECT setval(pg_get_serial_sequence('users', 'id'), COALESCE((SELECT MAX(id) FROM users), 1));
SELECT setval(pg_get_serial_sequence('products', 'id'), COALESCE((SELECT MAX(id) FROM products), 1));
SELECT setval(pg_get_serial_sequence('reviews', 'id'), COALESCE((SELECT MAX(id) FROM reviews), 1));
SELECT setval(pg_get_serial_sequence('orders', 'id'), COALESCE((SELECT MAX(id) FROM orders), 1));
SELECT setval(pg_get_serial_sequence('order_items', 'id'), COALESCE((SELECT MAX(id) FROM order_items), 1));
