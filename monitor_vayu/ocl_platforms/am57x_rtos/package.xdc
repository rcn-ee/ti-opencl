/*
 *  Copyright (c) 2016 by Texas Instruments and others.
 *  All rights reserved. This program and the accompanying materials
 *  are made available under the terms of the Eclipse Public License v1.0
 *  which accompanies this distribution, and is available at
 *  http://www.eclipse.org/legal/epl-v10.html
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 *
 * */

/*
 *  ======== package.xdc ========
 *
 */
/* requires ti.catalog.arp32;        */
requires ti.catalog.c6000;
/* requires ti.catalog.arm.cortexm4; */
requires ti.catalog.arm.cortexa15;
requires xdc.platform [1,0,1];

/*!
 *  ======== ocl_platforms.am57x_rtos ========
 *  Platform package for the OpenCL RTOS on DRA7XX SDP, copied and modified
 *  from ti.platforms.evmDRA7XX.
 *
 *  This package implements the interfaces (xdc.platform.IPlatform)
 *  necessary to build and run executables on the DRA74XX SDP platform.
 *
 *  @a(Throws)
 *  `XDCException` exceptions are thrown for fatal errors. The following error
 *  codes are reported in the exception message:
 *  @p(dlist)
 *      -  `ti.platforms.evmDRA74XX.LINK_TEMPLATE_ERROR`
 *           This error is raised when this platform cannot found the default
 *           linker command template `linkcmd.xdt` in the build target's
 *           package. When a target does not contain this file, the config
 *           parameter `{@link xdc.cfg.Program#linkTemplate}` must be set.
 *  @p
 */
package ocl_platforms.am57x_rtos [1,0,0,0] {
    module Platform;
}
/*
 *  Copied and modifed from the following platform for OpenCL RTOS, 5-5-2016.
 *  @(#) ti.platforms.evmDRA7XX; 1, 0, 0, 0,; 1-29-2016 10:02:10; /db/ztree/library/trees/platform/platform-q17/src/
 */

