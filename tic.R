get_stage ("after_success") %>%
    add_step (step_hello_world ()) %>%
    add_step (step_run_covr ( exclusions = "src/sqlite3/sqlite3.c"))
