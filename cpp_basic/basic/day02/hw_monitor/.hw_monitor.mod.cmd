savedcmd_hw_monitor.mod := printf '%s\n'   hw_monitor.o | awk '!x[$$0]++ { print("./"$$0) }' > hw_monitor.mod
