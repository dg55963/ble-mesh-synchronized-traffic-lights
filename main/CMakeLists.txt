set(srcs "main.c"
        "board.c"
        "ble_mesh_config.c"
        "my_timers.c"
        "mesh_transport.c"
        "node_state.c"
        "node_logic.c"
        "mesh_handlers.c"
       )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS  ".")
