echo ondemand | sudo tee /sys/bus/cpu/devices/cpu[046]/cpufreq/scaling_governor
echo dmc_ondemand | sudo tee /sys/class/devfreq/dmc/governor
echo simple_ondemand | sudo tee /sys/class/devfreq/fb000000.gpu/governor
echo rknpu_ondemand | sudo tee /sys/class/devfreq/fdab0000.npu/governor
