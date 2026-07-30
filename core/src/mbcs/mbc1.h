/*

A15 A14 A13   Address range        Region
 0   0   0    $0000-$1FFF          RAM Enable                           register Ram "gate?"
 0   0   1    $2000-$3FFF          ROM Bank Number (low bits)
 0   1   0    $4000-$5FFF          RAM Bank / ROM Bank (high bits)
 0   1   1    $6000-$7FFF          Banking Mode Select
 1   0   1    $A000-$BFFF          External RAM access
 
 */