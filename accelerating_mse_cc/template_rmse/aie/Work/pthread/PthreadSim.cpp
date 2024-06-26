#include "systemc.h"
#include <hls_stream_thread_safe.h>
#include <ap_axi_sdata.h>
#include <map>
#include <thread>
#include "adf/new_frontend/adf.h"
#include "adf/x86sim/x86sim.h"
#include "adf/buffer_port/buffer_port.h"
#include "adf/adf_api/X86SimConfig.h"
// adf::headers of all kernels
#include "src/my_kernel_1.h"
#include "../common/common.h"

////// Kernel function Wrapper declarations //////
void b0_kernel_wrapper(x86sim::stream_internal *, x86sim::stream_internal *, x86sim::stream_internal *);
void b1_kernel_wrapper(x86sim::stream_internal *, x86sim::stream_internal *, x86sim::stream_internal *);

////// Class kernel dtor wrapper extern declaration //////

////// Class kernel ctor wrapper extern declaration //////

////// Kernel Inits extern declaration //////

namespace {
} // namespace
namespace x86sim
{

////// Kernel Classes //////

class Kernel_b0_my_kernel_function : public IMEKernel
{
public: 
    Kernel_b0_my_kernel_function(ISimulator *sim, std::string label)
    : IMEKernel(sim, label)
    {
    }

protected: 
    virtual void invokeKernel() override
    {
        b0_kernel_wrapper(
          ((IStreamConnector*) input(0)) -> stream(),
          ((IStreamConnector*) input(1)) -> stream(),
          ((IStreamConnector*) output(0)) -> stream()
        );
    }
};

class Kernel_b1_sum_kernels : public IMEKernel
{
public: 
    Kernel_b1_sum_kernels(ISimulator *sim, std::string label)
    : IMEKernel(sim, label)
    {
    }

protected: 
    virtual void invokeKernel() override
    {
        b1_kernel_wrapper(
          ((IStreamConnector*) input(0)) -> stream(),
          ((IStreamConnector*) input(1)) -> stream(),
          ((IStreamConnector*) output(0)) -> stream()
        );
    }
};

////// Set Initial Value for input async RTP //////

static void initValue(IRtpConnector* rtp, int8_t* val, size_t bytes)
{
    updateRtp(val, bytes,rtp);
}

ISimulator *createSimulator(ISimulator::Kind kind)
{
    ISimulatorConfig simConfig = {};
    simConfig._enableProgress = true;
    simConfig._enableEventTrace = std::getenv("X86SIM_EVENT_TRACE_ON");
    simConfig._enableEventTracePrint = std::getenv("X86SIM_EVENT_TRACE_PRINT_ON");
    simConfig._enableSnapshots = std::getenv("X86SIM_SNAPSHOTS_ON");
    simConfig._simTimeout = 0;
    simConfig._plWaitTime = 0;
    simConfig._meKernelIters = 0;

    if (ISimulator::Kind::kBasic == kind)
    {
        auto simParams = createSimParams();
        simParams -> meKernelIters = -1 /* aiecompiler argument value */ ;
        if (!simParams -> populate())
            std::exit(EXIT_FAILURE);
        simConfig._simTimeout = simParams->simTimeout;
        simConfig._plWaitTime = simParams->plWaitTime;
        simConfig._meKernelIters = simParams->meKernelIters;
        simConfig._enableStopOnDeadlock = std::getenv("X86SIM_STOP_ON_DEADLOCK");
        simConfig._enableHandShakeWithExtTb = std::getenv("X86SIM_HANDSHAKE_EXT_TB");
        simConfig._socketIO = false;
        if (auto env = std::getenv("PACKAGEDIR"))
            simConfig._optionsFile = env + std::string("/options/x86sim.options");
    }
    if (ISimulator::Kind::kSwemu == kind)
    {
        simConfig._meKernelIters = -1;
        if (auto optionFilePath = std::getenv("X86SIM_OPTIONSPATH"))
            simConfig._optionsFile = optionFilePath ;
    }
    if (simConfig._simTimeout == 0)
        if (auto val = std::getenv("X86SIM_TIMEOUT"))
            simConfig._simTimeout = std::atoi(val);

    if (!(simConfig._optionsFile).empty())
        ISimulatorOptions::processFile(simConfig);

    if (auto inputDir = std::getenv("INPUTDIR"))
        simConfig._inputDir = inputDir;
    if (auto outputDir = std::getenv("OUTPUTDIR"))
        simConfig._outputDir = outputDir;
    if ((simConfig._inputDir).empty())
        simConfig._inputDir = ".";
    if ((simConfig._outputDir).empty())
        simConfig._outputDir = "./x86simulator_output";

    ISimulator *sim = SimulatorFactory::simulator(simConfig, kind);
    IBasicSimulator *basicSim = dynamic_cast<IBasicSimulator*>(sim);
    ISwemuSimulator *swemuSim = dynamic_cast<ISwemuSimulator*>(sim);
    IXmcSimulator *xmcSim = dynamic_cast<IXmcSimulator*>(sim);
    IStreamConnector *scon_i3_po0_i0_pi0
        = ConnectorFactory::streamConnector(sim, "scon_i3_po0_i0_pi0");
    sim -> addConnectorMetaData(
        {scon_i3_po0_i0_pi0, "stream", "uint8", "aie_graph.in_1.out[0]", "aie_graph.my_kernel_1.in[0]", "out", "in", 1, 1});
    IStreamConnector *scon_i4_po0_i0_pi1
        = ConnectorFactory::streamConnector(sim, "scon_i4_po0_i0_pi1");
    sim -> addConnectorMetaData(
        {scon_i4_po0_i0_pi1, "stream", "uint8", "aie_graph.in_2.out[0]", "aie_graph.my_kernel_1.in[1]", "out", "in", 1, 1});
    IStreamConnector *scon_i5_po0_i1_pi0
        = ConnectorFactory::streamConnector(sim, "scon_i5_po0_i1_pi0");
    sim -> addConnectorMetaData(
        {scon_i5_po0_i1_pi0, "stream", "uint8", "aie_graph.in_3.out[0]", "aie_graph.my_kernel_2.in[0]", "out", "in", 1, 1});
    IStreamConnector *scon_i6_po0_i1_pi1
        = ConnectorFactory::streamConnector(sim, "scon_i6_po0_i1_pi1");
    sim -> addConnectorMetaData(
        {scon_i6_po0_i1_pi1, "stream", "uint8", "aie_graph.in_4.out[0]", "aie_graph.my_kernel_2.in[1]", "out", "in", 1, 1});
    IStreamConnector *scon_i2_po0_i7_pi0
        = ConnectorFactory::streamConnector(sim, "scon_i2_po0_i7_pi0");
    sim -> addConnectorMetaData(
        {scon_i2_po0_i7_pi0, "stream", "float", "aie_graph.my_kernel_sum.out[0]", "aie_graph.out_1.in[0]", "out", "in", 1, 1});
    IStreamConnector *scon_i0_po0_i2_pi0
        = ConnectorFactory::streamConnector(sim, "scon_i0_po0_i2_pi0");
    sim -> addConnectorMetaData(
        {scon_i0_po0_i2_pi0, "stream", "float", "aie_graph.my_kernel_1.out[0]", "aie_graph.my_kernel_sum.in[0]", "out", "in", 1, 1});
    IStreamConnector *scon_i1_po0_i2_pi1
        = ConnectorFactory::streamConnector(sim, "scon_i1_po0_i2_pi1");
    sim -> addConnectorMetaData(
        {scon_i1_po0_i2_pi1, "stream", "float", "aie_graph.my_kernel_2.out[0]", "aie_graph.my_kernel_sum.in[1]", "out", "in", 1, 1});
    // Graph configs
    // {id, name, test-iterations, x86SimPtr}
    DFGraph *gr_aie_graph = new DFGraph(sim);
    sim->addGraphConfig({ 0, "aie_graph", -1, gr_aie_graph});
    Kernel_b0_my_kernel_function *ker_i0
        = new Kernel_b0_my_kernel_function(sim, "ker_i0");
    gr_aie_graph -> addKernel(ker_i0);
    ker_i0 -> addInput(scon_i3_po0_i0_pi0);
    ker_i0 -> addInput(scon_i4_po0_i0_pi1);
    ker_i0 -> addOutput(scon_i0_po0_i2_pi0);
    sim -> addNodeMetaData({ker_i0, "aie_graph.my_kernel_1", {
        {scon_i0_po0_i2_pi0, {"out[0]"}}
        , {scon_i3_po0_i0_pi0, {"in[0]"}}
        , {scon_i4_po0_i0_pi1, {"in[1]"}}
    }});

    Kernel_b0_my_kernel_function *ker_i1
        = new Kernel_b0_my_kernel_function(sim, "ker_i1");
    gr_aie_graph -> addKernel(ker_i1);
    ker_i1 -> addInput(scon_i5_po0_i1_pi0);
    ker_i1 -> addInput(scon_i6_po0_i1_pi1);
    ker_i1 -> addOutput(scon_i1_po0_i2_pi1);
    sim -> addNodeMetaData({ker_i1, "aie_graph.my_kernel_2", {
        {scon_i1_po0_i2_pi1, {"out[0]"}}
        , {scon_i5_po0_i1_pi0, {"in[0]"}}
        , {scon_i6_po0_i1_pi1, {"in[1]"}}
    }});

    Kernel_b1_sum_kernels *ker_i2
        = new Kernel_b1_sum_kernels(sim, "ker_i2");
    gr_aie_graph -> addKernel(ker_i2);
    ker_i2 -> addInput(scon_i0_po0_i2_pi0);
    ker_i2 -> addInput(scon_i1_po0_i2_pi1);
    ker_i2 -> addOutput(scon_i2_po0_i7_pi0);
    sim -> addNodeMetaData({ker_i2, "aie_graph.my_kernel_sum", {
        {scon_i0_po0_i2_pi0, {"in[0]"}}
        , {scon_i1_po0_i2_pi1, {"in[1]"}}
        , {scon_i2_po0_i7_pi0, {"out[0]"}}
    }});

    if (basicSim)
    {
        INode *platformIn_i3
            = NodeFactory::fileReader
            (sim, "platformIn_i3",
            simConfig._inputDir + "/data/in_plio_source_1.txt", 
            false, UINT8, 16, false);
        platformIn_i3 -> addOutput(scon_i3_po0_i0_pi0);

        INode *platformIn_i4
            = NodeFactory::fileReader
            (sim, "platformIn_i4",
            simConfig._inputDir + "/data/in_plio_source_2.txt", 
            false, UINT8, 16, false);
        platformIn_i4 -> addOutput(scon_i4_po0_i0_pi1);

        INode *platformIn_i5
            = NodeFactory::fileReader
            (sim, "platformIn_i5",
            simConfig._inputDir + "/data/in_plio_source_3.txt", 
            false, UINT8, 16, false);
        platformIn_i5 -> addOutput(scon_i5_po0_i1_pi0);

        INode *platformIn_i6
            = NodeFactory::fileReader
            (sim, "platformIn_i6",
            simConfig._inputDir + "/data/in_plio_source_4.txt", 
            false, UINT8, 16, false);
        platformIn_i6 -> addOutput(scon_i6_po0_i1_pi1);

        INode *platformOut_i7
            = NodeFactory::fileWriter
            (sim, "platformOut_i7",
            simConfig._outputDir + "/data/out_plio_sink_1.txt", 
            false, FLOAT, 4, false);
        platformOut_i7 -> addInput(scon_i2_po0_i7_pi0);


        auto configs = basicSim->getConfig();
        adf::initializeX86SimConfigurations(
            std::get<0>(configs),
            std::get<1>(configs),
            std::get<2>(configs),
            std::get<3>(configs),
            std::get<4>(configs),
            std::get<5>(configs),
            std::get<6>(configs),
            std::get<7>(configs),
            std::get<8>(configs),
            std::get<9>(configs));
    }
    if (swemuSim)
    {
        IPlatformStreamNode *platformIn_i3
            = NodeFactory::streamReader(sim, "platformIn_i3", false);
        swemuSim->registerHlsStreamNode(platformIn_i3, "in_plio_1");
        platformIn_i3 -> addOutput(scon_i3_po0_i0_pi0);

        IPlatformStreamNode *platformIn_i4
            = NodeFactory::streamReader(sim, "platformIn_i4", false);
        swemuSim->registerHlsStreamNode(platformIn_i4, "in_plio_2");
        platformIn_i4 -> addOutput(scon_i4_po0_i0_pi1);

        IPlatformStreamNode *platformIn_i5
            = NodeFactory::streamReader(sim, "platformIn_i5", false);
        swemuSim->registerHlsStreamNode(platformIn_i5, "in_plio_3");
        platformIn_i5 -> addOutput(scon_i5_po0_i1_pi0);

        IPlatformStreamNode *platformIn_i6
            = NodeFactory::streamReader(sim, "platformIn_i6", false);
        swemuSim->registerHlsStreamNode(platformIn_i6, "in_plio_4");
        platformIn_i6 -> addOutput(scon_i6_po0_i1_pi1);

        IPlatformStreamNode *platformOut_i7
            = NodeFactory::streamWriter(sim, "platformOut_i7", false);
        swemuSim->registerHlsStreamNode(platformOut_i7, "out_plio_1");
        platformOut_i7 -> addInput(scon_i2_po0_i7_pi0);

    }
    if (xmcSim)
    {
        IPlatformBuffer * bufplatformIn_i3
            = PlatformBufferFactory::inputBuffer(sim, "bufplatformIn_i3", 32768);
        INode *platformIn_i3
            = NodeFactory::bufferReader(sim, "platformIn_i3", bufplatformIn_i3, 1, 16);
        platformIn_i3 -> addOutput(scon_i3_po0_i0_pi0);

        IPlatformBuffer * bufplatformIn_i4
            = PlatformBufferFactory::inputBuffer(sim, "bufplatformIn_i4", 32768);
        INode *platformIn_i4
            = NodeFactory::bufferReader(sim, "platformIn_i4", bufplatformIn_i4, 1, 16);
        platformIn_i4 -> addOutput(scon_i4_po0_i0_pi1);

        IPlatformBuffer * bufplatformIn_i5
            = PlatformBufferFactory::inputBuffer(sim, "bufplatformIn_i5", 32768);
        INode *platformIn_i5
            = NodeFactory::bufferReader(sim, "platformIn_i5", bufplatformIn_i5, 1, 16);
        platformIn_i5 -> addOutput(scon_i5_po0_i1_pi0);

        IPlatformBuffer * bufplatformIn_i6
            = PlatformBufferFactory::inputBuffer(sim, "bufplatformIn_i6", 32768);
        INode *platformIn_i6
            = NodeFactory::bufferReader(sim, "platformIn_i6", bufplatformIn_i6, 1, 16);
        platformIn_i6 -> addOutput(scon_i6_po0_i1_pi1);

        IPlatformBuffer * bufplatformOut_i7
            = PlatformBufferFactory::outputBuffer(sim, "bufplatformOut_i7", 32768);
        INode *platformOut_i7
            = NodeFactory::bufferWriter(sim, "platformOut_i7", bufplatformOut_i7, 4, 4);
        platformOut_i7 -> addInput(scon_i2_po0_i7_pi0);

    }
    if ( !xmcSim)
    {
        sim->start();
    }
    return sim;
}

void *createBasicSimulatorInstance()
{
    static auto  g_sim = std::unique_ptr<ISimulator>
        (createSimulator(ISimulator::Kind::kBasic));
    return g_sim.get();
}

void *createSwemuSimulatorInstance()
{
    static auto  g_sim = std::unique_ptr<ISimulator>
        (createSimulator(ISimulator::Kind::kSwemu));
    return dynamic_cast<x86sim::ISwemuSimulator*>(g_sim.get());
}

void *createXmcSimulator()
{
    auto sim = createSimulator(ISimulator::Kind::kXmcGraph);
    return dynamic_cast<x86sim::IXmcSimulator*>(sim);
}

}  //end of x86Sim namespace


#include "adf/x86sim/symbolVisibility.h"
#include "adf/x86sim/x86simSwemuIfc.hpp"

