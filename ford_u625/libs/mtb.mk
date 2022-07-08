################################################
# Auto-generated as part of running getlibs 
################################################

# List of shared libraries 
SEARCH_TARGET_CY8CKIT-149=../mtb_shared/TARGET_CY8CKIT-149/latest-v1.X
SEARCH_capsense=../mtb_shared/capsense/latest-v3.X
SEARCH_core-lib=../mtb_shared/core-lib/latest-v1.X
SEARCH_core-make=../mtb_shared/core-make/release-v1.9.0
SEARCH_emeeprom=../mtb_shared/emeeprom/release-v2.10.0
SEARCH_mtb-hal-cat2=../mtb_shared/mtb-hal-cat2/latest-v2.X
SEARCH_mtb-pdl-cat2=../mtb_shared/mtb-pdl-cat2/latest-v1.X
SEARCH_recipe-make-cat2=../mtb_shared/recipe-make-cat2/latest-v1.X

# Shared libraries added to build 
SEARCH+=$(SEARCH_TARGET_CY8CKIT-149)
SEARCH+=$(SEARCH_capsense)
SEARCH+=$(SEARCH_core-lib)
SEARCH+=$(SEARCH_core-make)
SEARCH+=$(SEARCH_emeeprom)
SEARCH+=$(SEARCH_mtb-hal-cat2)
SEARCH+=$(SEARCH_mtb-pdl-cat2)
SEARCH+=$(SEARCH_recipe-make-cat2)
-include $(CY_INTERNAL_APP_PATH)/importedbsp.mk
