savedcmd_mem_health_check.mod := printf '%s\n'   mem_health_check.o | awk '!x[$$0]++ { print("./"$$0) }' > mem_health_check.mod
