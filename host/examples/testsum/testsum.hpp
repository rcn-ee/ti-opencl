#include <cmath>
#include <algorithm>
/*-----------------------------------------------------------------------------
* The following defines specialized templates to provide a string
* containing the typename
*----------------------------------------------------------------------------*/
template<class T>
struct TypeName 
{
    string getName();

  private:
    T *t; 
};

template<> string TypeName<double>::getName()        {return(string("double")); }
template<> string TypeName<float>::getName()         {return(string("float")); }
template<> string TypeName<unsigned long>::getName() {return(string("ulong"));}
template<> string TypeName<long>::getName()          {return(string("long")); }
template<> string TypeName<unsigned int>::getName()  {return(string("uint"));}
template<> string TypeName<int>::getName()           {return(string("int")); }
template<> string TypeName<unsigned char>::getName() {return(string("uchar"));}
template<> string TypeName<char>::getName()          {return(string("char")); }

/*-----------------------------------------------------------------------------
* specification of the OclTest template
*----------------------------------------------------------------------------*/
template <typename TYPE1>
class OclTest 
{
    private:
        cl::Kernel kernel;
        string     myType;
        cl::Event  event;

        /*---------------------------------------------------------------------
        * variables for the test
        *--------------------------------------------------------------------*/
        int vecsize;

    public:
        cl::Event *getEventPtr() { return &event;}
        OclTest() {}

        OclTest( cl::CommandQueue& queue, const char* kernelFile, 
                int argc, char *argv[])
        {
            cl::Device device   = queue.getInfo<CL_QUEUE_DEVICE>();
            cl::Context context = queue.getInfo<CL_QUEUE_CONTEXT>();

            cout << "---------- building OpenCL kernel (" 
                 << kernelFile << ") -----" << endl;

            myType = TypeName<TYPE1>().getName();
            cout << "   My type is " << myType.c_str() << endl;

            /*-----------------------------------------------------------------
            * Demonstrate using defines in the ocl build
            *----------------------------------------------------------------*/
            string buildOptions;

            { 
                /*-------------------------------------------------------------
                * create preprocessor defines for the kernel
                *------------------------------------------------------------*/
                char buf[256]; 
                sprintf(buf,"-D TYPE1=%s ", myType.c_str());
                buildOptions += string(buf);
            }

            /*-----------------------------------------------------------------
            * build the program from the source in the file
            *----------------------------------------------------------------*/
            ifstream file(kernelFile);
            string prog((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());


            cl::Program::Sources source(1, make_pair(prog.c_str(),
                                         prog.length()+1));
            cl::Program program(context, source);
            file.close();

            try 
            {
                cerr << "   buildOptions " << buildOptions << endl;
                cl::vector<cl::Device> foo;
                foo.push_back(device);
                program.build(foo, buildOptions.c_str() );
            } 
            catch(cl::Error& err) 
            {
                /*-------------------------------------------------------------
                * Get the build log
                *------------------------------------------------------------*/
                cerr << "Build failed! " << err.what() 
                     << '(' << err.err() << ')' << endl;
                cerr << "retrieving  log ... " << endl;
                cerr << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
                     << endl;
                exit(-1);
            }

            /*-----------------------------------------------------------------
            * Get the name of the kernel from the filename
            *----------------------------------------------------------------*/
            string kernelName = string(kernelFile)
                                  .substr(0,string(kernelFile).find(".cl"));
            kernel = cl::Kernel(program, kernelName.c_str());
        }

        inline void initData(int _vecsize)
        {
            vecsize = _vecsize;
        }

        inline cl::Kernel& getKernel() { return(kernel); }

        /*---------------------------------------------------------------------
        * Methods to return information for queuing work-groups
        *--------------------------------------------------------------------*/
        cl::NDRange getGlobalWorkItems() 
        {
            return ( cl::NDRange( vecsize ) ); 
        }

        cl::NDRange getWorkItemsInWorkGroup() 
        {
            /*-----------------------------------------------------------------
            * Only one work item per workgroup
            *----------------------------------------------------------------*/
            return ( cl::NDRange(vecsize) ); 
        }
};
