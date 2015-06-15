Python OpenCL Bindings
**********************

There is a python OpenCL binding called pyopencl. Using pyopencl, you can use all the scripting and existing libraries of python in combination with the power of compute offload DSPs on an HP m800 cartridge.

To enable pyopencl on the m800, you will first need to ensure that you can communicate through your firewall by setting the proxy environment variables. If you are not behind a firewall, then this step is not needed.

.. code-block:: bash

    export http_proxy=<proxy:port>
    export https_proxy=<proxy:port>

Then the following commands are used to download, build and install pyopencl and its prerequisites

.. code-block:: bash

    sudo -E apt-get install python-pip python-dev ti-opencl
    sudo -E pip install mako numpy pyopencl
    sudo -E pip install six --upgrade

After completion of the above steps, pyopencl should be installed and ready on your system. You can verify that everything is operational by creating an example with the following content and then running it.

.. code-block:: python

    #!/usr/bin/python
    import pyopencl as cl
    kcode = """kernel void test() { printf("Hello from DSP (%d)\\n", get_group_id(0)); }"""
    ctx   = cl.create_some_context()
    Q     = cl.CommandQueue(ctx)
    prg   = cl.Program(ctx, kcode).build(options="")
    prg.test(Q, [8], [1]).wait()

Your output should look similar to the below, however note that the order of the prints and what core they are on will vary from run to run.

::

    [core 2] Hello from DSP (0)
    [core 1] Hello from DSP (2)
    [core 7] Hello from DSP (1)
    [core 4] Hello from DSP (5)
    [core 5] Hello from DSP (6)
    [core 0] Hello from DSP (7)
    [core 3] Hello from DSP (4)
    [core 6] Hello from DSP (3)

The generic PyOpenCL documentation can be found `here <http://documen.tician.de/pyopencl/>`__.
