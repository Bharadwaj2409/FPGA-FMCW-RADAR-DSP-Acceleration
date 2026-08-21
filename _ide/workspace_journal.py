# 2026-08-21T18:23:03.456117500
import vitis

client = vitis.create_client()
client.set_workspace(path="project_1")

platform = client.get_component(name="ADC_Platform")
status = platform.build()

comp = client.get_component(name="ADC_Capture")
comp.build()

