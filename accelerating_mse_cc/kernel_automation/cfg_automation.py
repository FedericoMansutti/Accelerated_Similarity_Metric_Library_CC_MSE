def build_cfg(kernel_count):
    # Original static part of the config
    static_config = '''

[connectivity]
nk = setup_aie:1:setup_aie_0
nk = sink_from_aie:1:sink_from_aie_0

slr = setup_aie_0:SLR0
slr = sink_from_aie_0:SLR0

sp = sink_from_aie_0.m_axi_gmem1:MC_NOC0
sp = setup_aie_0.m_axi_gmem0:MC_NOC0
'''

    # Generating dynamic stream_connect entries
    stream_connect_entries = []
    for i in range(1, kernel_count * 2 + 1):
        stream_connect_entries.append(f'stream_connect = setup_aie_0.s_{i}:ai_engine_0.in_plio_{i}')

    # AI Engine output to sink_from_aie
    stream_connect_entries.append('stream_connect = ai_engine_0.out_plio_1:sink_from_aie_0.input_stream')

    # Joining all stream connect entries
    stream_connect_section = '\n'.join(stream_connect_entries)

    # Adding Vivado section
    vivado_section = '''\n
[vivado]
# use following line to improve the hw_emu running speed affected by platform
prop=fileset.sim_1.xsim.elaborate.xelab.more_options={-override_timeprecision -timescale=1ns/1ps}
'''

    # Combining all parts together
    config_content = f'{static_config}\n{stream_connect_section}\n{vivado_section}'
    
    return config_content
