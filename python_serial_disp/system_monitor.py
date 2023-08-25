from serialDisplay import Display
from time import sleep
import psutil
import GPUtil
from datetime import datetime

time_date = datetime.now()

### Definitions ###
port = "/dev/ttyUSB0"
baud = 2000000

wallpaper_img = ""
color_background = (0x10c5)
color_clock_text = (0xffff)
color_date_text  = (0xffff)

# color_Bar_fg     = (0x2de0)
color_Bar_fg     = (0xe765)
color_Bar_bg     = (0x9492)
color_stat_text  = (0xffff)
color_stat_val   = (0xffff)

color_cpu_bar_fg = (0x35bf)
color_cpu_bar_bg = (0x29e9)
color_cpu_circle = (0x08a4)
color_cpu_text_1 = (0xffff)
color_cpu_text_2 = (0xffff)

color_gpu_bar_fg = (0x89df)
color_gpu_bar_bg = (0x29e9)
color_gpu_circle = (0x08a4)
color_gpu_text_1 = (0xffff)
color_gpu_text_2 = (0xffff)

###-------------###

### Variables ###
cpu_use   = 0
gpu_use   = 30
cpu_temp  = 0
gpu_temp  = 30
ram_used  = 0
fan_speed = 2000

cpu_max = 100
gpu_max = 100
fan_max = 7500
total_ram = 0 
disp_bckl = 255
###-------------###

# ### Previous values ###
# cpu_prv_angle = 0
# gpu_prv_angle = 0
# cpu_prv_tem   = 0
# gpu_prv_tem   = 0
# fan_speed_prv = 0
# ram_prv_used  = 0
# ###-------------###

disp = Display(port, baud)

def init():
    disp.setBacklight(disp_bckl)
    disp.setRotation(3)
    ### Draw background ###
    disp.drawToSprite()
    disp.fillScreen(color_background)
    disp.setFont(0)
    disp.setTextColor(color_cpu_text_2, color_cpu_circle)
    draw_cpu_bg()
    draw_gpu_bg()
    draw_status_base()
    disp.pushSprite()

# get the needed info
# get the wallpaper 

### Helper Functions ###
def draw_cpu_bg():
    disp.setColor(color_cpu_bar_bg)
    disp.setBgColor(color_background)
    disp.fillSmoothCircle(54, 67, 44) # bar background
    
    disp.setColor(color_cpu_circle)
    disp.setBgColor(color_cpu_bar_bg)
    disp.fillSmoothCircle(54, 67, 35) # center circle

    disp.setFont(0)
    disp.setTextColor(color_cpu_text_2, color_cpu_circle)
    disp.printText(36, 78, "CPU")

def draw_gpu_bg():
    disp.setColor(color_gpu_bar_bg)
    disp.setBgColor(color_background)
    disp.fillSmoothCircle(54, 172, 44) # bar background
    
    disp.setColor(color_gpu_circle)
    disp.setBgColor(color_gpu_bar_bg)
    disp.fillSmoothCircle(54, 172, 35) # center circle

    disp.setFont(0)
    disp.setTextColor(color_cpu_text_2, color_cpu_circle)
    disp.printText(36, 183, "GPU")

def draw_cpu_bar(cpu_use):
    cpu_end = (cpu_use/100.0)*360.0
    if(cpu_end == 0):
        cpu_end = 1
    # erase previous
    draw_cpu_bg()

    disp.setColor(color_cpu_bar_fg)
    disp.setBgColor(color_cpu_bar_bg)
    disp.drawSmoothArc(54, 67, 44, 35, 0, int(cpu_end), 0)

    disp.setFont(1)
    disp.setTextColor(color_cpu_text_1, color_cpu_circle)
    str_cpu_use = str(int(cpu_use)) + "%"
    if(cpu_use<10):
        str_cpu_use = "0" + str(int(cpu_use)) + "%"
    disp.printText(32, 50, str_cpu_use)

def draw_gpu_bar(gpu_use):
    gpu_end = (gpu_use/100.0)*360.0
    if(gpu_end == 0):
        gpu_end = 1
    # erase previous
    draw_gpu_bg()

    disp.setColor(color_gpu_bar_fg)
    disp.setBgColor(color_gpu_bar_bg)
    disp.drawSmoothArc(54, 172, 44, 35, 0, int(gpu_end), 0)

    disp.setFont(1)
    disp.setTextColor(color_gpu_text_1, color_gpu_circle)
    str_gpu_use = str(gpu_use) + "%"
    if(gpu_use<10):
        str_gpu_use = "0" + str(int(gpu_use)) + "%"
    disp.printText(32,155, str_gpu_use)


def draw_date_time():
    time_date = datetime.now()
    time_str = time_date.strftime("%H:%M")
    date_str = time_date.strftime("%b %d")
    disp.setColor(color_background)
    disp.fillRectangle(222, 183, 150, 52)
    disp.setFont(2)
    disp.setTextColor(color_clock_text, color_background)
    disp.printText(230, 184, time_str)

    disp.setFont(0)
    disp.setTextColor(color_date_text, color_background)
    disp.printText(258, 218, date_str)

def draw_status_base_bar():
    disp.setColor(color_Bar_bg)
    disp.fillRectangle(119, 38, 175, 6)
    disp.fillRectangle(119, 73, 175, 6)
    disp.fillRectangle(119, 108, 175, 6)
    disp.fillRectangle(119, 143, 175, 6)


def draw_status_base():
    draw_status_base_bar()

    disp.setFont(0)
    disp.setTextColor(color_stat_text, color_background)
    disp.printText(119, 20, "CPU TEMP")
    disp.printText(119, 55, "GPU TEMP")
    disp.printText(119, 90, "Fan SPEED")
    disp.printText(119, 125, "RAM USED")

def update_status_bar():
    draw_status_base_bar()
    cpu_val = min(cpu_temp, cpu_max)
    gpu_val = min(gpu_temp, gpu_max)
    fan_val = min(fan_speed, fan_max)
    ram_val = min(ram_used, total_ram)

    disp.setColor(color_Bar_fg)
    disp.fillRectangle(119, 38, int((cpu_val/cpu_max)*175.0), 6)
    disp.fillRectangle(119, 73, int((gpu_val/gpu_max)*175.0), 6)
    disp.fillRectangle(119, 108, int((fan_val/fan_max)*175.0), 6)
    disp.fillRectangle(119, 143, int((ram_val/total_ram)*175.0), 6)

    disp.setFont(0)
    disp.setTextColor(color_stat_val, color_background)
    # TODO: calculate x pos
    cpu_tmp_x = 270
    gpu_tmp_x = 270
    fan_x = 270
    ram_x = 270
    disp.printText(cpu_tmp_x, 20, str(cpu_temp) + "°")
    disp.printText(gpu_tmp_x, 55, str(gpu_temp) + "°")
    disp.printText(fan_x, 90, str(int(fan_val/fan_max*100)) + "%")
    disp.printText(ram_x, 125, str(int((ram_val/total_ram)*100)) + "%")


gpu_dummy = 0
gpuDum_flip = True
def updateGpuDummy():
    global gpu_dummy
    global gpuDum_flip

    if(gpuDum_flip):
        gpu_dummy += 8
    else:
        gpu_dummy -= 8
    if(gpu_dummy < 1):
        gpuDum_flip = not gpuDum_flip
        gpu_dummy = 1
    if(gpu_dummy > 99):
        gpuDum_flip = not gpuDum_flip
        gpu_dummy = 99

    return gpu_dummy

if __name__ == "__main__":
    ### initialize everything ###
    init()
    disp.setBacklight(50)

    ### Update in loop ###
    while True:
        cpu_use = psutil.cpu_percent()
        total_ram = psutil.virtual_memory().total
        ram_used = psutil.virtual_memory().available
        temperature = psutil.sensors_temperatures().get('coretemp')
        cpu_temp = temperature[0].current

        gpu_use = updateGpuDummy()

        disp.drawToSprite()
        draw_cpu_bar(cpu_use)
        draw_gpu_bar(gpu_use)
        draw_date_time()
        update_status_bar()
        disp.pushSprite()
        sleep(0.5)