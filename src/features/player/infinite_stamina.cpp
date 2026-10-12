#include "player.hpp"
#include "../../modules/hooks/hook.hpp"  // IWYU pragma: keep
#include "../../modules/ui/config.hpp"   // IWYU pragma: keep
#include "../../modules/utils/logger.hpp"

namespace Features
{
  namespace Player
  {
    void InitializeStamina() { Logger::Log("[InfiniteStamina] Mono hook disabled for stability."); }
  }  // namespace Player
}  // namespace Features
