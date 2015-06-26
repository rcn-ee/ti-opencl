*****************************************
EdmaMgr
*****************************************
.. c:type:: EdmaMgr
    typedef void *EdmaMgr_Handle;

    An opaque type used to hold EdmaMgr Handle data structures.

.. c:function:: EdmaMgr_Handle EdmaMgr_alloc(int32_t max_linked_transfers)

    Allocate an EdmaMgr Handle for use with all other EdmaMgr APIs.  The
    max_linked_transfers argument should be 1 for a handle used with the single
    transfer EdmaMgr APIs.  It can be greater than one for handles used with the
    multiple transfer APIs.

.. c:function:: int32_t EdmaMgr_free(EdmaMgr_Handle h)

    Free the resources associated with the EdmaMgr Handle.

.. c:function:: void EdmaMgr_wait(EdmaMgr_Handle h)

    Wait for all edma transfers associated with the handle h to complete before
    continuing this thread of execution.

Single Transfer EdmaMgr APIs
===============================

This group of EdmaMgr functions will initiate an edma operation on one source
and destination addresses.

.. c:function:: int32_t EdmaMgr_copy1D1D(EdmaMgr_Handle h, void *src, void *dst, int32_t num_bytes)

.. c:function:: int32_t EdmaMgr_copy1D2D(EdmaMgr_Handle h, void *src, void *dst, int32_t num_bytes, int32_t num_lines, int32_t pitch)

.. c:function:: int32_t EdmaMgr_copy2D1D(EdmaMgr_Handle h, void *src, void *dst, int32_t num_bytes, int32_t num_lines, int32_t pitch)

.. c:function:: int32_t EdmaMgr_copy2D2D (EdmaMgr_Handle h, void *src, void *dst, int32_t num_bytes, int32_t num_lines, int32_t pitch)

.. c:function:: int32_t EdmaMgr_copy2D2DSep (EdmaMgr_Handle h, void *src, void *dst, int32_t num_bytes, int32_t num_lines, int32_t src_pitch, int32_t dst_pitch)

.. c:function:: int32_t EdmaMgr_copyFast (EdmaMgr_Handle h, void *src, void *dst)

    Repeat the last EdmaMgr command using this handle, but with different src
    and dst address.  


Multiple Transfer EdmaMgr APIs
===============================

This group of EdmaMgr functions will initiate edma operations on a set of  source
and destination address pairs.  

.. c:function:: int32_t EdmaMgr_copy1D1DLinked (EdmaMgr_Handle h, void *src[], void *dst[], int32_t num_bytes[], int32_t num_transfers)

.. c:function:: int32_t EdmaMgr_copy1D2DLinked (EdmaMgr_Handle h, void *src[], void *dst[], int32_t num_bytes[], int32_t num_lines[], int32_t pitch[], int32_t num_transfers)

.. c:function:: int32_t EdmaMgr_copy2D1DLinked (EdmaMgr_Handle h, void *src[], void *dst[], int32_t num_bytes[], int32_t num_lines[], int32_t pitch[], int32_t num_transfers)

.. c:function:: int32_t EdmaMgr_copy2D2DLinked (EdmaMgr_Handle h, void *src[], void *dst[], int32_t num_bytes[], int32_t num_lines[], int32_t pitch[], int32_t num_transfers)

.. c:function:: int32_t EdmaMgr_copy2D2DSepLinked(EdmaMgr_Handle h, void *src[], void *dst[], int32_t num_bytes[], int32_t num_lines[], int32_t src_pitch[], int32_t dst_pitch[], int32_t num_transfers)

.. c:function:: int32_t EdmaMgr_copyLinkedFast (EdmaMgr_Handle h, void *src[], void *dst[], int32_t num_transfers) 


