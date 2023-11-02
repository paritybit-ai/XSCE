#include "oprf_psi/oprf_psi.h"

namespace oprf_psi_offline {
namespace offline {
bool USE_OFFLINE_CHANNEL = false;
std::vector<uint8_t> recv_data;
uint64_t cur_read_recv_data{0};
std::vector<uint8_t> send_data;
uint64_t cur_write_send_data{0};
}
}