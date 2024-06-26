#include <iostream>
#include "adf/new_frontend/adf.h"
#include "adf/adf_api/X86SimConfig.h"


/************************** Graph Configurations  *****************************/

  adf::X86SimGraphConfig GraphConfigurations[] = {
  // {id, name, runForIter, x86SimPtr}
    {0, "aie_graph", -1, nullptr},
  };
  const int NUM_GRAPH = 1;

/************************** PLIO Configurations  *****************************/

  adf::X86SimPLIOConfig PLIOConfigurations[] = {
  //{id, name, loginal_name}
    {0, "aie_graph.in_1", "in_plio_1"},
    {1, "aie_graph.in_2", "in_plio_2"},
    {2, "aie_graph.in_3", "in_plio_3"},
    {3, "aie_graph.in_4", "in_plio_4"},
    {4, "aie_graph.out_1", "out_plio_1"},
  };
  const int NUM_PLIO = 5;


/************************** ADF API initializer *****************************/

  class InitializeAIEControlXRT
  {
  public:
    InitializeAIEControlXRT()
    {
      std::cout<<"Initializing ADF API..."<<std::endl;
      adf::initializeX86SimConfigurations(GraphConfigurations, NUM_GRAPH,
                                    nullptr, 0,
                                    nullptr, 0,
                                    PLIOConfigurations, NUM_PLIO,
                                    nullptr, 0);
    }
  } initAIEControlXRT;



#if !defined(__CDO__)

// Kernel Stub Definition
  void my_kernel_function(input_stream<unsigned char> *,input_stream<unsigned char> *,output_stream<int> *) { /* Stub */ } 
  void sum_kernels(input_stream<int> *,input_stream<int> *,output_stream<float> *) { /* Stub */ } 
#endif
