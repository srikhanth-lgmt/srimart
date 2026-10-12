#include "ai/MockChatProvider.h"
#include <algorithm>
#include <cctype>

namespace srikanth::srikanthmart {

static std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

MockChatProvider::MockChatProvider() {
    faq_db_["order status"] = "You can track your order status live in the 'My Orders' section. Order stages are: Pending -> Confirmed -> Shipped -> Delivered.";
    faq_db_["track"] = "To track any shipment, navigate to 'My Orders' where our live status workflow updates as sellers dispatch your items.";
    faq_db_["shipping"] = "SrikanthMart offers fast express shipping across all major regions. Standard delivery takes 2 to 4 business days.";
    faq_db_["delivery"] = "Standard delivery typically arrives in 2-4 business days. Free shipping is provided for all orders exceeding INR 500.";
    faq_db_["return"] = "Our hassle-free return policy gives you 14 days from delivery to request a replacement or full refund on eligible goods.";
    faq_db_["refund"] = "Refunds are processed to your original payment method within 3-5 business days upon item inspection.";
    faq_db_["seller"] = "Interested in selling? Create an account with the 'Seller' role to instantly list products, track orders, and view sales analytics!";
    faq_db_["payment"] = "SrikanthMart supports secure instant mock payments via Mock UPI, Mock Card, and Mock NetBanking.";
    faq_db_["review"] = "Buyers can leave 1-to-5 star ratings and reviews on products they have purchased once an order is completed.";
    faq_db_["discount"] = "Special seasonal discounts and promotions are automatically reflected at checkout on select products.";
    faq_db_["laptop"] = "We offer the UltraBook Pro 15 with 16GB RAM and 512GB NVMe SSD for INR 45,000. Check the Electronics category!";
    faq_db_["mobile"] = "Check out our Nexus 5G Smartphone with 120Hz AMOLED display for INR 18,000 in the catalog!";
    faq_db_["owner"] = "SrikanthMart is developed and maintained under the leadership of Srikanth.";
    faq_db_["contact"] = "Need further support? You can reach our administrative desk at support@srikanthmart.com or ask me here anytime!";
}

std::string MockChatProvider::GetReply(const std::string& user_message,
                                     const std::string& /*context*/) {
    std::string lower = ToLower(user_message);

    for (const auto& [keyword, answer] : faq_db_) {
        if (lower.find(keyword) != std::string::npos) {
            return answer;
        }
    }

    return "Hello! I am SrikanthMart AI Assistant. I can assist you with tracking orders, discovering products (laptops, mobiles, audio), checking return policies, or selling on our marketplace. How may I help you today?";
}

} // namespace srikanth::srikanthmart
