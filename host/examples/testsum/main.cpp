#define PROFILING              // Define to see the time the kernel takes
#define __NO_STD_VECTOR        // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS // needed for exceptions
#include <CL/cl.hpp>
#include <fstream>
#include <iostream>
using namespace std;

#include "testsum.hpp"

int main(int argc, char* argv[])
{
    if ( argc < 5) 
    {
        cerr << "Use: {cpu|dsp|both} kernelFile nvec vecsize" << endl;
        exit(EXIT_FAILURE);
    }

    /*-------------------------------------------------------------------------
    * handle command-line arguments
    *------------------------------------------------------------------------*/
    const std::string platformName(argv[1]);
    const char* kernelFile = argv[2];

    int nvec    = atoi(argv[3]);
    int vecsize = atoi(argv[4]);

    cl::vector<int> deviceType;
    cl::vector< cl::CommandQueue > contextQueues;

    /*-------------------------------------------------------------------------
    * crudely parse the command line arguments
    *------------------------------------------------------------------------*/
    if (platformName.compare("cpu")==0)
        deviceType.push_back(CL_DEVICE_TYPE_CPU);

    else if(platformName.compare("dsp")==0) 
        deviceType.push_back(CL_DEVICE_TYPE_ACCELERATOR);

    else if(platformName.compare("both")==0) 
    {
        deviceType.push_back(CL_DEVICE_TYPE_ACCELERATOR);
        deviceType.push_back(CL_DEVICE_TYPE_CPU);
    } 
    else 
    { cerr << "Invalid device type!" << endl; return(1); }


    /*-------------------------------------------------------------------------
    * create the contexts and queues
    *------------------------------------------------------------------------*/
    try 
    {
        cl::vector< cl::Platform > platformList;
        cl::Platform::get(&platformList);

        /*---------------------------------------------------------------------
        * Get all the appropriate devices for the platform the implementation 
        * thinks we should be using.  find the user-specified devices
        *--------------------------------------------------------------------*/
        cl::vector<cl::Device> devices;

        for (int i=0; i < deviceType.size(); i++) 
        {
            cl::vector<cl::Device> dev;
            platformList[0].getDevices(deviceType[i], &dev);
            for(int j=0; j < 1 /*dev.size()*/; j++) devices.push_back(dev[j]);
        }

        /*---------------------------------------------------------------------
        * set a single context
        *--------------------------------------------------------------------*/
        cl_context_properties cprops[] = {CL_CONTEXT_PLATFORM, 0, 0};
        cl::Context context(devices, cprops);

        cout << "Using the following device(s) in one context" << endl;
        for (int i=0; i < devices.size(); i++)  
            cout << "  " << devices[i].getInfo<CL_DEVICE_NAME>() << endl;

        /*---------------------------------------------------------------------
        * Create the separate command queues to perform work
        *--------------------------------------------------------------------*/
        cl::vector< cl::CommandQueue > contextQueues;
        for (int i=0; i < devices.size(); i++)  
        {
#ifdef PROFILING
            cl::CommandQueue queue(context, devices[i], CL_QUEUE_PROFILING_ENABLE);
#else
            cl::CommandQueue queue(context, devices[i], 0);
#endif
            contextQueues.push_back( queue );
        }

        /*---------------------------------------------------------------------
        * Create tests for all the queues
        *--------------------------------------------------------------------*/
        cl::vector< OclTest<uint> > test;
        for (int i=0; i < contextQueues.size(); i++) 
            test.push_back(OclTest<uint>(contextQueues[i], kernelFile, argc-3, argv+3));

        int           nDevices = contextQueues.size();
        unsigned int* vec      = new uint[nvec * vecsize];
        int           vecBytes = vecsize * sizeof(uint);

        /*---------------------------------------------------------------------
        * Fill the host memory with random data for the sums
        *--------------------------------------------------------------------*/
        srand(0);
        for (int i=0; i < (nvec*vecsize); i++) vec[i] = (rand()&0xffffff);

        /*---------------------------------------------------------------------
        * Create a separate buffer for each device in the context
        *--------------------------------------------------------------------*/
#ifdef USE_MAP
        /*---------------------------------------------------------------------
        * This maps all of the host data into memory so it does not need
        * to be manually copied.
        *--------------------------------------------------------------------*/
        cl::vector< cl::Buffer > d_vec;
        for (int i=0; i < contextQueues.size(); i++) 
            d_vec.push_back(cl::Buffer(context, CL_MEM_COPY_HOST_PTR, 
                        nvec* vecBytes, vec) );

        int vecOffset = vecBytes; // the buffer is of size vec, so use row offset
#else
        cl::vector< cl::Buffer > d_vec;
        for (int i=0; i < contextQueues.size(); i++)
            d_vec.push_back(cl::Buffer(context, CL_MEM_READ_WRITE, vecBytes) );

        int vecOffset = 0; // the buffer is the size of one vector so no offset
#endif

        /*---------------------------------------------------------------------
        * run the tests
        *--------------------------------------------------------------------*/
#pragma omp parallel for
        for (int i=0; i < contextQueues.size(); i++) 
        {
            test[i].initData(vecsize);
            test[i].getKernel().setArg(0,vecsize);
            test[i].getKernel().setArg(1,d_vec[i]);

            for(int j=i; j < nvec; j += nDevices) 
            {
#ifdef USE_MAP
                test[i].getKernel().setArg(2,j); // set the offset for the kernel
#else
                test[i].getKernel().setArg(2,0);
                contextQueues[i].enqueueWriteBuffer(d_vec[i], CL_TRUE,0, vecBytes, &vec[j*vecsize]);
#endif
                contextQueues[i].enqueueNDRangeKernel(
                        test[i].getKernel(), 
                        cl::NullRange,                      // offset starts at 0,0
                        test[i].getGlobalWorkItems(),       // number of work groups
                        test[i].getWorkItemsInWorkGroup(),  // workgroup size
                        NULL, test[i].getEventPtr());

                /*-------------------------------------------------------------
                * manually transfer the data from the device
                *------------------------------------------------------------*/
                contextQueues[i].enqueueReadBuffer(d_vec[i], CL_TRUE,
                        j * vecOffset, vecBytes, &vec[j * vecsize]);
            }

            /*-----------------------------------------------------------------
            * wait for everything to finish
            *----------------------------------------------------------------*/
            contextQueues[i].finish();
        }

        /*---------------------------------------------------------------------
        * perform the golden test
        *--------------------------------------------------------------------*/
        {
            int i;
            srand(0);
            for (i=0; i < (nvec * vecsize); i++) 
            {
                unsigned int r = (rand() & 0xffffff);

                r += r;
                if (r != vec[i]) break;
            }

            if (i == (nvec * vecsize)) 
                 cout << "test passed" << endl;
            else cout << "TEST FAILED!" << endl;
        }
        delete [] vec;
    } 
    catch (cl::Error error) 
    { cerr << "caught exception: " << error.what() << '(' << error.err() << ')' << endl; }

    return EXIT_SUCCESS;
}
