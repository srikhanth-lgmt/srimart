-- V3__add_index_orders_status.sql
-- Optimizes order workflow queries by status

CREATE INDEX IF NOT EXISTS idx_orders_status ON orders(status);
