#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return (Wrap32)((static_cast<uint64_t>(zero_point.raw_value_) + n) % (1LL<<32));
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  static constexpr uint64_t TWO31 = 1UL << 31;
  static constexpr uint64_t TWO32 = 1UL << 32;

  auto const ckpt32 = wrap( checkpoint, zero_point );
  uint64_t const dis = raw_value_ - ckpt32.raw_value_;

  if ( dis <= TWO31 || checkpoint + dis < TWO32 ) {
    return checkpoint + dis;
  }
  return checkpoint + dis - TWO32;
}
