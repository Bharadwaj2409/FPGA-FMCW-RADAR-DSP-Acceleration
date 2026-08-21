# 2026-08-21T17:23:28.921871300
import vitis

client = vitis.create_client()
client.set_workspace(path="project_1")

platform = client.create_platform_component(name = "ADC_Platform",hw_design = "$COMPONENT_LOCATION/../ADC_Wrapper.xsa",os = "standalone",cpu = "ps7_cortexa9_0",domain_name = "standalone_ps7_cortexa9_0",compiler = "gcc")

platform = client.get_component(name="ADC_Platform")
status = platform.build()

status = platform.update_hw(hw_design = "$COMPONENT_LOCATION/../ADC1_Wrapper.xsa")

status = platform.build()

comp = client.create_app_component(name="hello_world",platform = "$COMPONENT_LOCATION/../ADC_Platform/export/ADC_Platform/ADC_Platform.xpfm",domain = "standalone_ps7_cortexa9_0",template = "hello_world")

status = platform.build()

comp = client.get_component(name="hello_world")
comp.build()

status = platform.build()

comp.build()

comp = client.get_component(name="hello_world")
comp.set_app_config(key = "USER_COMPILE_DEFINITIONS", values = ["-m"])

comp = client.get_component(name="hello_world")
status = comp.clean()

status = platform.build()

comp.build()

comp = client.get_component(name="hello_world")
comp.set_app_config(key = "USER_COMPILE_DEFINITIONS", values = ["-m"])

comp.set_app_config(key = "USER_COMPILE_DEFINITIONS", values = [""])

comp.set_app_config(key = "USER_COMPILE_DEFINITIONS", values = ["m"])

comp = client.get_component(name="hello_world")
status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

comp = client.get_component(name="hello_world")
comp.set_app_config(key = "USER_COMPILE_DEFINITIONS", values = ["m", "lm"])

comp = client.get_component(name="hello_world")
status = comp.clean()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

status = platform.build()

comp.build()

vitis.dispose()

