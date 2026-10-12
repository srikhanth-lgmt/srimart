/**
 * SrikanthMart — Native C++ Marketplace Frontend Application Logic
 * AJAX Client communicating with native Drogon backend /api/v1 endpoints
 * Owner: Srikanth
 */

const app = {
    state: {
        activeRole: null,
        activeUser: null,
        products: [],
        currentCategory: '',
        searchQuery: '',
        cart: { items: [], total_amount_cents: 0, formatted_total: '₹0.00', total_items_count: 0 },
        currentDetailProduct: null,
        chatOpen: false
    },

    init: async function() {
        console.log("Initializing SrikanthMart App...");
        this.checkHealth();
        await this.refreshSession();
        await this.loadProducts();
        await this.loadCart();
        this.updateCartBadge();
    },

    refreshSession: async function() {
        try {
            const response = await fetch('/api/v1/auth/me', { credentials: 'same-origin' });
            const json = await response.json();
            this.setCurrentUser(json.success ? json.data : null);
        } catch (_) { this.setCurrentUser(null); }
    },

    setCurrentUser: function(user) {
        this.state.activeUser = user;
        this.state.activeRole = user?.role || null;
        document.getElementById('accountLabel').textContent = user ? `${user.name} · ${user.role}` : 'Browsing as guest';
        document.getElementById('authButton').hidden = !!user;
        document.getElementById('logoutButton').hidden = !user;
        document.getElementById('tabSeller').hidden = user?.role !== 'SELLER';
        document.getElementById('tabAdmin').hidden = user?.role !== 'ADMIN';
        document.getElementById('tabOrders').hidden = user?.role !== 'BUYER';
    },

    openAuth: function() { this.showAuthForm('login'); this.openModal('authModal'); },
    togglePassword: function(id) {
        const input = document.getElementById(id);
        if (!input) return;
        input.type = input.type === 'password' ? 'text' : 'password';
        input.nextElementSibling.textContent = input.type === 'password' ? 'Show password' : 'Hide password';
    },
    showAuthForm: function(which) {
        document.getElementById('loginForm').hidden = which !== 'login';
        document.getElementById('registerForm').hidden = which !== 'register';
        document.getElementById('authTitle').textContent = which === 'login' ? 'Welcome back' : 'Create your account';
    },
    login: async function(event) {
        event.preventDefault();
        const button = document.getElementById('loginSubmit');
        const error = document.getElementById('loginError');
        button.disabled = true; button.textContent = 'Signing in…'; error.textContent = '';
        try {
            const response = await fetch('/api/v1/auth/login', { method: 'POST', credentials: 'same-origin', headers: {'Content-Type':'application/json'}, body: JSON.stringify({email: document.getElementById('loginEmail').value, password: document.getElementById('loginPassword').value}) });
            const json = await response.json();
            if (!response.ok || !json.success) throw new Error(json.error?.message || 'Unable to sign in. Check your details.');
            this.setCurrentUser(json.data); this.closeModal('authModal'); await this.loadCart();
            const view = json.data.role === 'ADMIN' ? 'admin' : json.data.role === 'SELLER' ? 'seller' : 'catalog';
            this.navigate(view); this.showToast('Signed in successfully', 'success');
        } catch (e) { error.textContent = e.message; }
        finally { button.disabled = false; button.textContent = 'Log in'; }
    },
    register: async function(event) {
        event.preventDefault();
        const password = document.getElementById('registerPassword').value;
        if (password !== document.getElementById('registerConfirmPassword').value) { document.getElementById('registerError').textContent = 'Passwords do not match.'; return; }
        if (!document.getElementById('acceptTerms').checked) { document.getElementById('registerError').textContent = 'Please accept the terms and conditions.'; return; }
        if (password.length < 8) { document.getElementById('registerError').textContent = 'Use a password with at least 8 characters.'; return; }
        const button = document.getElementById('registerSubmit');
        const error = document.getElementById('registerError');
        button.disabled = true; button.textContent = 'Creating account…'; error.textContent = '';
        try {
            const response = await fetch('/api/v1/auth/register', { method: 'POST', credentials: 'same-origin', headers: {'Content-Type':'application/json'}, body: JSON.stringify({name: document.getElementById('registerName').value, email: document.getElementById('registerEmail').value, password, role: document.getElementById('registerRole').value, accepted_terms: document.getElementById('acceptTerms').checked}) });
            const json = await response.json();
            if (!response.ok || !json.success) throw new Error(json.error?.message || 'Unable to create account.');
            this.showAuthForm('login'); document.getElementById('loginEmail').value = json.data.email;
            error.textContent = ''; this.showToast('Account created. Please log in.', 'success');
            document.getElementById('loginError').textContent = 'Your account is ready. Please log in.';
        } catch (e) { error.textContent = e.message; }
        finally { button.disabled = false; button.textContent = 'Create account'; }
    },
    logout: async function() {
        await fetch('/api/v1/auth/logout', {method:'POST', credentials:'same-origin'});
        this.setCurrentUser(null); this.state.cart = {items:[], total_amount_cents:0, formatted_total:'₹0.00', total_items_count:0};
        this.renderCart(); this.updateCartBadge(); this.navigate('catalog'); this.showToast('Signed out', 'success');
    },

    checkHealth: async function() {
        try {
            const res = await fetch('/api/v1/health');
            const data = await res.json();
            if (data.success && data.data.status === 'UP') {
                const el = document.getElementById('systemHealthBadge');
                if (el) el.innerHTML = `<i class="fa-solid fa-circle-check"></i> C++ Backend UP (${data.data.db})`;
            }
        } catch (e) {
            console.log("Health check status offline or local dev runner active");
        }
    },

    navigate: function(viewName) {
        const allowed = {seller: 'SELLER', admin: 'ADMIN', orders: 'BUYER'};
        if (allowed[viewName] && this.state.activeRole !== allowed[viewName]) {
            this.showToast('Please sign in with an account that has access.', 'error');
            if (!this.state.activeUser) this.openAuth();
            return;
        }
        // Update nav tabs
        document.querySelectorAll('.nav-tab').forEach(t => t.classList.remove('active'));
        if (viewName === 'catalog') document.getElementById('tabCatalog')?.classList.add('active');
        if (viewName === 'seller') document.getElementById('tabSeller')?.classList.add('active');
        if (viewName === 'admin') document.getElementById('tabAdmin')?.classList.add('active');
        if (viewName === 'orders') document.getElementById('tabOrders')?.classList.add('active');

        // Toggle sections
        document.querySelectorAll('.view-section').forEach(s => s.classList.remove('active'));
        const target = document.getElementById(viewName + 'View');
        if (target) target.classList.add('active');

        // Load view specific data
        if (viewName === 'catalog') this.loadProducts();
        if (viewName === 'seller') this.loadSellerDesk();
        if (viewName === 'admin') this.loadAdminPanel();
        if (viewName === 'orders') this.loadBuyerOrders();
    },

    // ==========================================
    // BUYER & CATALOG MODULE
    // ==========================================
    loadProducts: async function() {
        try {
            let url = `/api/v1/products?q=${encodeURIComponent(this.state.searchQuery)}`;
            if (this.state.currentCategory) {
                url += `&category=${encodeURIComponent(this.state.currentCategory)}`;
            }

            const res = await fetch(url);
            const json = await res.json();
            if (json.success) {
                this.state.products = json.data;
                this.renderProductsGrid(json.data);
            }
        } catch (e) {
            console.error("Failed to load products:", e);
        }
    },

    renderProductsGrid: function(products) {
        const grid = document.getElementById('productsGrid');
        if (!grid) return;

        if (products.length === 0) {
            grid.innerHTML = `
                <div style="grid-column: 1 / -1; text-align: center; padding: 48px; color: var(--text-muted);">
                    <i class="fa-solid fa-magnifying-glass" style="font-size: 2rem; margin-bottom: 12px;"></i>
                    <p>No products found matching your search criteria.</p>
                </div>
            `;
            return;
        }

        grid.innerHTML = products.map(p => {
            const formattedPrice = p.formatted_price || `₹${(p.price_cents / 100).toFixed(2)}`;
            return `
                <div class="product-card" onclick="app.openProductDetail(${p.id})">
                    <div class="card-img-box">
                        <img src="${p.image_url || 'https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600'}" alt="${p.name}">
                        <span class="card-tag">${p.category}</span>
                    </div>
                    <div class="card-content">
                        <h4>${p.name}</h4>
                        <p class="card-desc">${p.description}</p>
                        <div class="card-footer">
                            <div>
                                <div class="price-tag">${formattedPrice}</div>
                                <span class="stock-tag"><i class="fa-solid fa-check"></i> ${p.stock_qty} in stock</span>
                            </div>
                            <button class="btn btn-secondary btn-sm" onclick="event.stopPropagation(); app.quickAddToCart(${p.id})">
                                <i class="fa-solid fa-cart-plus"></i> Add
                            </button>
                        </div>
                    </div>
                </div>
            `;
        }).join('');
    },

    handleSearch: function(e) {
        this.state.searchQuery = e.target.value;
        if (e.key === 'Enter') {
            this.executeSearch();
        }
    },

    executeSearch: function() {
        this.state.searchQuery = document.getElementById('searchInput').value;
        this.loadProducts();
    },

    filterCategory: function(category) {
        this.state.currentCategory = category;
        document.querySelectorAll('.pill').forEach(pill => {
            if (pill.innerText.trim().toLowerCase() === (category || 'all').toLowerCase()) {
                pill.classList.add('active');
            } else {
                pill.classList.remove('active');
            }
        });
        const select = document.getElementById('categorySelect');
        if (select) select.value = category;
        this.loadProducts();
    },

    scrollToProducts: function() {
        document.getElementById('catalogView')?.scrollIntoView({ behavior: 'smooth' });
    },

    // ==========================================
    // PRODUCT DETAIL & REVIEWS (F8)
    // ==========================================
    openProductDetail: async function(productId) {
        try {
            const res = await fetch(`/api/v1/products/${productId}`);
            const json = await res.json();
            if (json.success) {
                const p = json.data;
                this.state.currentDetailProduct = p;

                document.getElementById('detailImg').src = p.image_url || 'https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600';
                document.getElementById('detailTitle').innerText = p.name;
                document.getElementById('detailCategory').innerText = p.category;
                document.getElementById('detailPrice').innerText = p.formatted_price || `₹${(p.price_cents / 100).toFixed(2)}`;
                document.getElementById('detailStock').innerText = `${p.stock_qty} units available`;
                document.getElementById('detailDesc').innerText = p.description;

                this.loadProductReviews(productId);
                this.openModal('productDetailModal');
            }
        } catch (e) {
            console.error("Failed to load details:", e);
        }
    },

    loadProductReviews: async function(productId) {
        const list = document.getElementById('detailReviewsList');
        if (!list) return;

        try {
            const res = await fetch(`/api/v1/products/${productId}/reviews`);
            const json = await res.json();
            if (json.success && json.data.length > 0) {
                list.innerHTML = json.data.map(r => `
                    <div class="review-item">
                        <div style="display: flex; justify-content: space-between; margin-bottom: 4px;">
                            <strong>${r.reviewer_name || 'Buyer'}</strong>
                            <span class="text-gold">${'★'.repeat(r.rating)}${'☆'.repeat(5 - r.rating)}</span>
                        </div>
                        <p style="color: var(--text-secondary); margin: 0;">"${r.comment}"</p>
                    </div>
                `).join('');
            } else {
                list.innerHTML = `<p class="text-muted" style="font-size: 0.85rem;">No reviews yet. Be the first verified buyer to leave feedback!</p>`;
            }
        } catch (e) {
            list.innerHTML = `<p class="text-muted">Could not load reviews.</p>`;
        }
    },

    submitReview: async function() {
        if (!this.state.currentDetailProduct) return;
        const rating = parseInt(document.getElementById('newReviewRating').value);
        const comment = document.getElementById('newReviewComment').value.trim();

        if (!comment) {
            this.showToast("Please enter your review comment", "error");
            return;
        }

        try {
            const res = await fetch(`/api/v1/products/${this.state.currentDetailProduct.id}/reviews`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ rating, comment })
            });
            const json = await res.json();
            if (json.success) {
                this.showToast("Review submitted successfully!", "success");
                document.getElementById('newReviewComment').value = '';
                this.loadProductReviews(this.state.currentDetailProduct.id);
            } else {
                this.showToast(json.error?.message || "Reviews require verified purchase of completed order", "error");
            }
        } catch (e) {
            this.showToast("Error submitting review", "error");
        }
    },

    addCurrentToCart: function() {
        if (this.state.currentDetailProduct) {
            this.quickAddToCart(this.state.currentDetailProduct.id);
            this.closeModal('productDetailModal');
        }
    },

    // ==========================================
    // SHOPPING CART & CHECKOUT (F4, F5)
    // ==========================================
    loadCart: async function() {
        try {
            const res = await fetch('/api/v1/cart', {
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                this.state.cart = json.data;
                this.renderCart();
                this.updateCartBadge();
            }
        } catch (e) {
            console.error("Failed to load cart:", e);
        }
    },

    quickAddToCart: async function(productId) {
        try {
            const res = await fetch('/api/v1/cart/items', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ product_id: productId, quantity: 1 })
            });
            const json = await res.json();
            if (json.success) {
                this.state.cart = json.data;
                this.renderCart();
                this.updateCartBadge();
                this.showToast("Item added to cart", "success");
            } else {
                this.showToast(json.error?.message || "Failed to add to cart", "error");
            }
        } catch (e) {
            this.showToast("Error communicating with server", "error");
        }
    },

    updateCartQuantity: async function(productId, qty) {
        try {
            const res = await fetch(`/api/v1/cart/items/${productId}`, {
                method: 'PUT',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ quantity: qty })
            });
            const json = await res.json();
            if (json.success) {
                this.state.cart = json.data;
                this.renderCart();
                this.updateCartBadge();
            }
        } catch (e) {
            this.showToast("Error updating cart", "error");
        }
    },

    removeCartItem: async function(productId) {
        try {
            const res = await fetch(`/api/v1/cart/items/${productId}`, {
                method: 'DELETE',
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                this.state.cart = json.data;
                this.renderCart();
                this.updateCartBadge();
                this.showToast("Item removed from cart", "success");
            }
        } catch (e) {
            this.showToast("Error removing item", "error");
        }
    },

    renderCart: function() {
        const container = document.getElementById('cartItemsContainer');
        const totalEl = document.getElementById('cartDrawerTotal');
        if (!container || !totalEl) return;

        totalEl.innerText = this.state.cart.formatted_total || `₹${(this.state.cart.total_amount_cents / 100).toFixed(2)}`;

        if (!this.state.cart.items || this.state.cart.items.length === 0) {
            container.innerHTML = `
                <div style="text-align: center; padding: 48px 0; color: var(--text-muted);">
                    <i class="fa-solid fa-cart-shopping" style="font-size: 2.5rem; margin-bottom: 12px;"></i>
                    <p>Your shopping cart is empty.</p>
                </div>
            `;
            return;
        }

        container.innerHTML = this.state.cart.items.map(item => `
            <div class="cart-item-card">
                <img src="${item.product_image_url || 'https://images.unsplash.com/photo-1526170375885-4d8ecf77b99f?w=600'}" alt="${item.product_name}">
                <div class="cart-item-info">
                    <h5>${item.product_name}</h5>
                    <small>${item.formatted_unit_price || ('₹' + (item.unit_price_cents / 100).toFixed(2))}</small>
                    <div class="qty-control">
                        <button class="qty-btn" onclick="app.updateCartQuantity(${item.product_id}, ${item.quantity - 1})">-</button>
                        <span>${item.quantity}</span>
                        <button class="qty-btn" onclick="app.updateCartQuantity(${item.product_id}, ${item.quantity + 1})">+</button>
                    </div>
                </div>
                <button class="btn btn-danger btn-sm" onclick="app.removeCartItem(${item.product_id})"><i class="fa-solid fa-trash"></i></button>
            </div>
        `).join('');
    },

    updateCartBadge: function() {
        const badge = document.getElementById('cartCount');
        if (badge) {
            badge.innerText = this.state.cart.total_items_count || 0;
        }
    },

    toggleCartModal: function() {
        const drawer = document.getElementById('cartDrawer');
        const overlay = document.getElementById('cartOverlay');
        drawer?.classList.toggle('open');
        if (overlay) {
            overlay.style.display = drawer?.classList.contains('open') ? 'block' : 'none';
        }
    },

    openCheckoutModal: function() {
        if (!this.state.cart.items || this.state.cart.items.length === 0) {
            this.showToast("Your cart is empty!", "error");
            return;
        }
        document.getElementById('checkoutTotalAmount').innerText = this.state.cart.formatted_total;
        this.toggleCartModal();
        this.openModal('checkoutModal');
    },

    handleMockCheckout: async function(e) {
        e.preventDefault();
        const address = document.getElementById('checkoutAddress').value.trim();
        const paymentMethod = document.querySelector('input[name="paymentMethod"]:checked')?.value || 'MOCK_UPI';

        try {
            const res = await fetch('/api/v1/orders/checkout', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    shipping_address: address,
                    mock_payment_method: paymentMethod,
                    mock_payment_confirmed: true
                })
            });

            const json = await res.json();
            if (json.success) {
                this.closeModal('checkoutModal');
                this.showToast("Order placed successfully! Mock payment confirmed.", "success");
                await this.loadCart();
                this.navigate('orders');
            } else {
                this.showToast(json.error?.message || "Checkout failed", "error");
            }
        } catch (e) {
            this.showToast("Error processing checkout", "error");
        }
    },

    // ==========================================
    // ORDER HISTORY & STATUS WORKFLOW (F6, O2)
    // ==========================================
    loadBuyerOrders: async function() {
        const container = document.getElementById('buyerOrdersList');
        if (!container) return;

        try {
            const res = await fetch('/api/v1/orders/buyer', {
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                if (json.data.length === 0) {
                    container.innerHTML = `
                        <div class="content-panel" style="text-align: center; padding: 48px; color: var(--text-muted);">
                            <i class="fa-solid fa-box-open" style="font-size: 2.5rem; margin-bottom: 12px;"></i>
                            <p>You have not placed any orders yet.</p>
                        </div>
                    `;
                    return;
                }

                container.innerHTML = json.data.map(order => {
                    const statusClass = `badge-${order.status.toLowerCase()}`;
                    return `
                        <div class="order-history-card">
                            <div class="order-card-header">
                                <div>
                                    <h4>Order #${order.id}</h4>
                                    <small class="text-muted">Placed: ${order.created_at || 'Recently'}</small>
                                </div>
                                <div>
                                    <span class="badge ${statusClass}"><i class="fa-solid fa-truck"></i> ${order.status}</span>
                                </div>
                            </div>
                            <div class="order-items-list">
                                ${order.items.map(it => `
                                    <div class="order-item-row">
                                        <span>${it.product_name} &times; ${it.quantity}</span>
                                        <strong>${it.formatted_unit_price || ('₹' + (it.unit_price_cents / 100).toFixed(2))}</strong>
                                    </div>
                                `).join('')}
                            </div>
                            <div style="display: flex; justify-content: space-between; align-items: center; border-top: 1px solid var(--border-subtle); padding-top: 12px;">
                                <span>Order Total:</span>
                                <strong style="font-size: 1.2rem; color: #fff;">${order.formatted_total}</strong>
                            </div>
                        </div>
                    `;
                }).join('');
            }
        } catch (e) {
            console.error("Failed to load buyer orders:", e);
        }
    },

    // ==========================================
    // SELLER DESK & DASHBOARD (F2, O3)
    // ==========================================
    loadSellerDesk: async function() {
        // 1. Load Seller Dashboard KPIs
        try {
            const res = await fetch('/api/v1/orders/seller/dashboard', {
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                document.getElementById('sellerKpiRevenue').innerText = json.data.formatted_revenue || '₹0.00';
                document.getElementById('sellerKpiOrders').innerText = json.data.total_orders_count || 0;
            }
        } catch (e) {
            console.error(e);
        }

        // 2. Load Seller Products
        try {
            const res = await fetch('/api/v1/products/seller/mine', {
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                document.getElementById('sellerKpiProducts').innerText = json.data.length;
                const tbody = document.getElementById('sellerProductsTable');
                if (tbody) {
                    tbody.innerHTML = json.data.map(p => `
                        <tr>
                            <td>
                                <strong>${p.name}</strong><br>
                                <small class="text-muted">ID: ${p.id}</small>
                            </td>
                            <td>${p.category}</td>
                            <td>${p.formatted_price}</td>
                            <td>${p.stock_qty}</td>
                            <td>
                                <button class="btn btn-secondary btn-sm" onclick="app.openEditProductModal(${p.id})"><i class="fa-solid fa-pen"></i></button>
                                <button class="btn btn-danger btn-sm" onclick="app.deleteProduct(${p.id})"><i class="fa-solid fa-trash"></i></button>
                            </td>
                        </tr>
                    `).join('');
                }
            }
        } catch (e) {
            console.error(e);
        }

        // 3. Load Incoming Orders for Seller
        try {
            const res = await fetch('/api/v1/orders/seller', {
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                const tbody = document.getElementById('sellerOrdersTable');
                if (tbody) {
                    tbody.innerHTML = json.data.map(o => `
                        <tr>
                            <td><strong>#${o.id}</strong></td>
                            <td>Buyer #${o.buyer_id}</td>
                            <td><span class="badge badge-${o.status.toLowerCase()}">${o.status}</span></td>
                            <td>${o.formatted_total}</td>
                            <td>${o.created_at || 'Today'}</td>
                            <td>
                                <select onchange="app.updateOrderStatus(${o.id}, this.value)" style="background: var(--bg-surface-elevated); color: #fff; padding: 4px; border: 1px solid var(--border-subtle); border-radius: 4px;">
                                    <option value="PENDING" ${o.status === 'PENDING' ? 'selected' : ''}>PENDING</option>
                                    <option value="CONFIRMED" ${o.status === 'CONFIRMED' ? 'selected' : ''}>CONFIRMED</option>
                                    <option value="SHIPPED" ${o.status === 'SHIPPED' ? 'selected' : ''}>SHIPPED</option>
                                    <option value="DELIVERED" ${o.status === 'DELIVERED' ? 'selected' : ''}>DELIVERED</option>
                                </select>
                            </td>
                        </tr>
                    `).join('');
                }
            }
        } catch (e) {
            console.error(e);
        }
    },

    updateOrderStatus: async function(orderId, nextStatus) {
        try {
            const res = await fetch(`/api/v1/orders/${orderId}/status`, {
                method: 'PUT',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ status: nextStatus })
            });
            const json = await res.json();
            if (json.success) {
                this.showToast(`Order #${orderId} status updated to ${nextStatus}`, 'success');
                this.loadSellerDesk();
            }
        } catch (e) {
            this.showToast("Failed to update status", "error");
        }
    },

    openNewProductModal: function() {
        document.getElementById('productFormTitle').innerText = "List New Marketplace Product";
        document.getElementById('formProductId').value = '';
        document.getElementById('formProductName').value = '';
        document.getElementById('formProductCategory').value = 'Electronics';
        document.getElementById('formProductPrice').value = '';
        document.getElementById('formProductStock').value = '25';
        document.getElementById('formProductImage').value = '';
        document.getElementById('formProductDesc').value = '';
        this.openModal('productFormModal');
    },

    openEditProductModal: function(productId) {
        const p = this.state.products.find(x => x.id === productId);
        if (!p) return;
        document.getElementById('productFormTitle').innerText = "Edit Product Listing";
        document.getElementById('formProductId').value = p.id;
        document.getElementById('formProductName').value = p.name;
        document.getElementById('formProductCategory').value = p.category;
        document.getElementById('formProductPrice').value = (p.price_cents / 100);
        document.getElementById('formProductStock').value = p.stock_qty;
        document.getElementById('formProductImage').value = p.image_url;
        document.getElementById('formProductDesc').value = p.description;
        this.openModal('productFormModal');
    },

    saveProductForm: async function(e) {
        e.preventDefault();
        const id = document.getElementById('formProductId').value;
        const name = document.getElementById('formProductName').value.trim();
        const category = document.getElementById('formProductCategory').value;
        const priceMajor = parseFloat(document.getElementById('formProductPrice').value);
        const priceCents = Math.round(priceMajor * 100);
        const stockQty = parseInt(document.getElementById('formProductStock').value);
        const imageUrl = document.getElementById('formProductImage').value.trim();
        const description = document.getElementById('formProductDesc').value.trim();

        const payload = {
            name,
            category,
            price_cents: priceCents,
            stock_qty: stockQty,
            image_url: imageUrl,
            description
        };

        const isEdit = !!id;
        const url = isEdit ? `/api/v1/products/${id}` : '/api/v1/products';
        const method = isEdit ? 'PUT' : 'POST';

        try {
            const res = await fetch(url, {
                method,
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(payload)
            });
            const json = await res.json();
            if (json.success) {
                this.closeModal('productFormModal');
                this.showToast(isEdit ? "Product updated successfully" : "Product listed successfully", "success");
                await this.loadProducts();
                this.loadSellerDesk();
            } else {
                this.showToast(json.error?.message || "Failed to save product", "error");
            }
        } catch (e) {
            this.showToast("Error saving product", "error");
        }
    },

    deleteProduct: async function(productId) {
        if (!confirm("Are you sure you want to delete this product listing?")) return;
        try {
            const res = await fetch(`/api/v1/products/${productId}`, {
                method: 'DELETE',
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                this.showToast("Product deleted", "success");
                await this.loadProducts();
                this.loadSellerDesk();
            } else {
                this.showToast(json.error?.message || "Could not delete product", "error");
            }
        } catch (e) {
            this.showToast("Error deleting product", "error");
        }
    },

    // ==========================================
    // ADMIN PANEL (F7)
    // ==========================================
    loadAdminPanel: async function() {
        // 1. Users list
        try {
            const res = await fetch('/api/v1/admin/users', {
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                const tbody = document.getElementById('adminUsersTable');
                if (tbody) {
                    tbody.innerHTML = json.data.map(u => `
                        <tr>
                            <td>#${u.id}</td>
                            <td><strong>${u.name}</strong></td>
                            <td>${u.email}</td>
                            <td><span class="badge ${u.role === 'ADMIN' ? 'badge-admin' : (u.role === 'SELLER' ? 'badge-confirmed' : 'badge-delivered')}">${u.role}</span></td>
                        </tr>
                    `).join('');
                }
            }
        } catch (e) {
            console.error(e);
        }

        // 2. All orders
        try {
            const res = await fetch('/api/v1/admin/orders', {
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                const tbody = document.getElementById('adminOrdersTable');
                if (tbody) {
                    tbody.innerHTML = json.data.map(o => `
                        <tr>
                            <td>#${o.id}</td>
                            <td>Buyer #${o.buyer_id}</td>
                            <td><span class="badge badge-${o.status.toLowerCase()}">${o.status}</span></td>
                            <td>${o.formatted_total}</td>
                            <td>${o.created_at || 'Recent'}</td>
                        </tr>
                    `).join('');
                }
            }
        } catch (e) {
            console.error(e);
        }

        // 3. Moderation table
        const modTbody = document.getElementById('adminModerationTable');
        if (modTbody) {
            modTbody.innerHTML = this.state.products.map(p => `
                <tr>
                    <td><strong>${p.name}</strong></td>
                    <td>Seller #${p.seller_id}</td>
                    <td>${p.category}</td>
                    <td>${p.formatted_price}</td>
                    <td>
                        <button class="btn btn-danger btn-sm" onclick="app.moderateRemoveProduct(${p.id})">
                            <i class="fa-solid fa-ban"></i> Remove Listing
                        </button>
                    </td>
                </tr>
            `).join('');
        }
    },

    moderateRemoveProduct: async function(productId) {
        if (!confirm("Admin Action: Remove this listing from marketplace?")) return;
        try {
            const res = await fetch(`/api/v1/admin/products/${productId}`, {
                method: 'DELETE',
                headers: { }
            });
            const json = await res.json();
            if (json.success) {
                this.showToast("Product listing removed by admin", "success");
                await this.loadProducts();
                this.loadAdminPanel();
            }
        } catch (e) {
            this.showToast("Error during moderation", "error");
        }
    },

    // ==========================================
    // AI CHATBOT INTEGRATION (O4)
    // ==========================================
    toggleChat: function() {
        const panel = document.getElementById('chatPanel');
        this.state.chatOpen = !this.state.chatOpen;
        if (panel) {
            panel.classList.toggle('open', this.state.chatOpen);
        }
    },

    openChat: function() {
        this.state.chatOpen = true;
        document.getElementById('chatPanel')?.classList.add('open');
    },

    askChatPrompt: function(text) {
        document.getElementById('chatInput').value = text;
        this.sendChatMessage();
    },

    sendChatMessage: async function() {
        const input = document.getElementById('chatInput');
        const message = input.value.trim();
        if (!message) return;

        // Render user message
        this.appendChatMessage(message, 'user');
        input.value = '';

        // Add loading bot message
        const loadingId = 'loading_' + Date.now();
        this.appendChatMessage('<i class="fa-solid fa-spinner fa-spin"></i> Thinking...', 'bot', loadingId);

        try {
            const res = await fetch('/api/v1/chat', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ message })
            });

            const json = await res.json();
            const loadingEl = document.getElementById(loadingId);
            if (loadingEl) {
                if (json.success) {
                    loadingEl.innerHTML = `<p>${json.data.reply}</p>`;
                } else {
                    loadingEl.innerHTML = `<p class="text-danger">${json.error?.message || "Service temporarily unavailable"}</p>`;
                }
            }
        } catch (e) {
            const loadingEl = document.getElementById(loadingId);
            if (loadingEl) {
                loadingEl.innerHTML = `<p>I am online in local mode! SrikanthMart features 2-4 day delivery, instant mock checkout, and full seller listing management.</p>`;
            }
        }
    },

    appendChatMessage: function(content, sender, elementId = null) {
        const container = document.getElementById('chatMessages');
        if (!container) return;

        const msgDiv = document.createElement('div');
        msgDiv.className = `chat-msg ${sender}`;
        if (elementId) msgDiv.id = elementId;
        msgDiv.innerHTML = `<p>${content}</p>`;

        container.appendChild(msgDiv);
        container.scrollTop = container.scrollHeight;
    },

    // ==========================================
    // MODAL & TOAST HELPERS
    // ==========================================
    openModal: function(modalId) {
        document.getElementById(modalId)?.classList.add('open');
    },

    closeModal: function(modalId) {
        document.getElementById(modalId)?.classList.remove('open');
    },

    showToast: function(message, type = 'info') {
        const container = document.getElementById('toastContainer');
        if (!container) return;

        const toast = document.createElement('div');
        toast.className = `toast toast-${type}`;
        toast.innerText = message;
        container.appendChild(toast);

        setTimeout(() => {
            toast.remove();
        }, 3200);
    }
};

// Initialize on DOM ready
document.addEventListener('DOMContentLoaded', () => {
    app.init();
});
