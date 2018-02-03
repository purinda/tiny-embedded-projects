import network
import gc
from global_config import RouterConfig

sta_if = network.WLAN(network.STA_IF)
sta_if.active(True)
sta_if.connect(RouterConfig.SSID, RouterConfig.PW) # Connect to an AP
gc.collect()
