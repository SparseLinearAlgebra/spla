#include "c_config.hpp"

spla_Status spla_Descriptor_make(spla_Descriptor* d) {
    auto descriptor = spla::Descriptor::make();
    *d              = as_ptr<spla_Descriptor_t>(descriptor.release());
    return SPLA_STATUS_OK;
}

spla_Status spla_Descriptor_set_struct_only(spla_Descriptor d, spla_bool value) {
    as_ptr<spla::Descriptor>(d)->set_struct_only(value != 0);
    return SPLA_STATUS_OK;
}

spla_Status spla_Descriptor_set_early_exit(spla_Descriptor d, spla_bool value) {
    as_ptr<spla::Descriptor>(d)->set_early_exit(value != 0);
    return SPLA_STATUS_OK;
}