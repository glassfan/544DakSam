connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292646055A"} -index 0
loadhw C:/VivadoProjects/ece544_project2_custom/ece544_project2_custom.sdk/n4fpga_hw_platform_0/system.hdf
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292646055A"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292646055A"} -index 0
dow C:/VivadoProjects/ece544_project2_custom/ece544_project2_custom.sdk/ece544_proj2_app/Debug/ece544_proj2_app.elf
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292646055A"} -index 0
con
