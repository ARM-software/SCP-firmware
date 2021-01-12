cmake_dependent_option(
    SCP_ENABLE_SCMI_RESET "Enable the scmi reset?"
    "${SCP_ENABLE_SCMI_RESET_INIT}" "DEFINED SCP_ENABLE_SCMI_RESET_INIT"
    "${SCP_ENABLE_SCMI_RESET}")
