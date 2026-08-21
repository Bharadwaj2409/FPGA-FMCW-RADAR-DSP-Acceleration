# 2026-08-21T17:58:50.838203600
import vitis

client = vitis.create_client()
client.set_workspace(path="project_1")

comp = client.get_component(name="hello_world")
comp.set_app_config(key = "USER_LINK_LIBRARIES", values = ["m"])

platform = client.get_component(name="ADC_Platform")
status = platform.build()

comp = client.get_component(name="hello_world")
comp.build()

status = platform.build()

comp.build()

comp = client.create_app_component(name="ADC_Capture",platform = "$COMPONENT_LOCATION/../ADC_Platform/export/ADC_Platform/ADC_Platform.xpfm",domain = "standalone_ps7_cortexa9_0",template = "hello_world")

client.delete_component(name="hello_world")

client.delete_component(name="componentName")

status = platform.build()

comp = client.get_component(name="ADC_Capture")
comp.build()

comp = client.get_component(name="ADC_Capture")
comp.set_app_config(key = "USER_LINK_LIBRARIES", values = ["m"])

status = platform.build()

comp = client.get_component(name="ADC_Capture")
comp.build()

vitis.dispose()

