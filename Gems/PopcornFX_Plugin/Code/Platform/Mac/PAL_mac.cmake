#----------------------------------------------------------------------------
# Copyright Persistant Studios, SARL. All Rights Reserved.
# https://www.popcornfx.com/terms-and-conditions/
#----------------------------------------------------------------------------

set(LY_PACKAGE_SERVER_URLS ${LY_PACKAGE_SERVER_URLS} "https://downloads.popcornfx.com/o3de-packages")
ly_associate_package(PACKAGE_NAME PopcornFX-${POPCORNFX_VERSION}-${POPCORNFX_LICENSE}-mac TARGETS PopcornFX PACKAGE_HASH 663694a31ab72ffa1876c8e6c0112c3b6e2dc033e0bf53f83d1db0ce48ac06e0)
set(PAL_TRAIT_POPCORNFX_SUPPORTED TRUE)