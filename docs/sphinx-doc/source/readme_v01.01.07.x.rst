OpenCL v01.01.07.x Readme
=========================

* Added the following extentions OpenCL C builtin functions. ::

    int      __cache_l2_32k(void);
    int      __cache_l2_64k(void);

    void*    __scratch_l2_start(void);
    uint32_t __scratch_l2_size(void);

* Modified the following functions to return an int rather than void.  They
  will return 1 on success and 0 on failure.  Calls that will shrink the cache
  size will always succeed.  Calls that increase the cache size may fail if the
  delta amount of memory dedicated to cache is not available based on other uses
  of L2 scrathpad memory. ::

    int      __cache_l1d_none  (void);
    int      __cache_l1d_all   (void);
    int      __cache_l1d_4k    (void);
    int      __cache_l1d_8k    (void);
    int      __cache_l1d_16k   (void);
    int      __cache_l2_none   (void);
    int      __cache_l2_32k    (void);
    int      __cache_l2_64k    (void);
    int      __cache_l2_128k   (void);
    int      __cache_l2_256k   (void);
    int      __cache_l2_512k   (void);

* These following standard C code, C6X intrinsics are now available from OpenCL
  C code.  Previously they were defined to operate on long long data types which
  are not available in OpenCL C.  In OpenCL C they are now applicable to the long
  or ulong data type.  ::

    long _labs       (long);
    long _lsadd      (long, int);
    long _lssub      (long, int);
    int  _sat        (long);
    uint _lnorm      (long);
    long _mpyidll    (int, int);
    uint _hill       (long);
    uint _loll       (long);
    long _itoll      (uint, uint);
    long _ldotp2     (int, int);
    long _mpy2ll     (int, int);
    long _mpyhill    (int, int);
    long _mpylill    (int, int);
    long _mpysu4ll   (int, uint);
    long _mpyu4ll    (uint, uint);
    long _smpy2ll    (int, int);
    long _addsub     (int, int);
    long _addsub2    (uint, uint);
    long _cmpy       (uint, uint);
    long _ddotph2    (long, uint);
    uint _ddotph2r   (long, uint);
    long _ddotpl2    (long, uint);
    uint _ddotpl2r   (long, uint);
    long _ddotp4     (uint, uint);
    long _dpack2     (uint, uint);
    long _dpackx2    (uint, uint);
    long _dmv        (uint, uint);
    long _mpy32ll    (int, int);
    long _mpy32su    (int, uint);
    long _mpy32us    (uint, int);
    long _mpy32u     (uint, uint);
    long _mpy2ir     (uint, int);
    long _saddsub    (int, int);
    long _saddsub2   (uint, uint);
    long _shfl3      (uint, uint);
    long _davgu4     (long, long);
    long _davg2      (long, long);
    long       & _mem8       (void *);
    long       & _amem8      (void *);
    const long & _mem8_const (const void *);
    const long & _amem8_const(const void *);

The above intrinsics are in addition to the already existing intrinsics below ::

    uint   _extu    (uint, uint, uint);
    int    _ext	    (int, uint, uint);
    uint   _set	    (uint, uint, uint);
    uint   _clr	    (uint, uint, uint);
    uint   _extur   (uint, int);
    int    _extr    (int, int);
    uint   _setr    (uint, int);
    uint   _clrr    (uint, int);
    int    _sadd    (int, int);
    int    _ssub    (int, int);
    int    _sshl    (int, uint);
    int    _add2    (int, int);
    int    _sub2    (int, int);
    uint   _subc    (uint, uint);
    uint   _lmbd    (uint, uint);
    int    _abs	    (int);
    uint   _norm    (int);
    int    _smpy    (int, int);
    int    _smpyhl  (int, int);
    int    _smpylh  (int, int);
    int    _smpyh   (int, int);
    int    _mpy	    (int, int);
    int    _mpyus   (uint, int);
    int    _mpysu   (int, uint);
    uint   _mpyu    (uint,  uint);
    int    _mpyhl   (int, int);
    int    _mpyhuls (uint, int);
    int    _mpyhslu (int, uint);
    uint   _mpyhlu  (uint, uint);
    int    _mpylh   (int, int);
    int    _mpyluhs (uint, int);
    int    _mpylshu (int, uint);
    uint   _mpylhu  (uint, uint);
    int    _mpyh    (int, int);
    int    _mpyhus  (uint, int);
    int    _mpyhsu  (int, uint);
    uint   _mpyhu   (uint, uint, uint);
    double _fabs    (double);
    float  _fabsf   (float);
    double _mpyid   (int, int);
    int    _spint   (float);
    int    _dpint   (double);
    float  _rcpsp   (float, float);
    double _rcpdp   (double, double);
    float  _rsqrsp  (float, float);
    double _rsqrdp  (double, double);
    uint   _hi	    (double);
    uint   _lo	    (double);
    double _itod    (uint, uint);
    float  _itof    (uint);
    uint   _ftoi    (float);
    int    _add4    (int, int);
    int    _avg2    (int, int);
    uint   _avgu4   (uint,  uint);
    int    _cmpeq2  (int, int);
    int    _cmpeq4  (int, int);
    int    _cmpgt2  (int, int);
    uint   _cmpgtu4 (uint, uint);
    int    _dotp2   (int, int);
    int    _dotpn2  (int, int);
    int    _dotpnrsu2(int, uint);
    int    _dotprsu2(int, uint);
    int    _dotpsu4 (int, uint);
    uint   _dotpu4  (uint, uint);
    int    _gmpy4   (int, int);
    int    _max2    (int, int);
    uint   _maxu4   (uint, uint);
    int    _min2    (int, int);
    uint   _minu4   (uint, uint);
    double _mpy2    (int, int);
    double _mpyhi   (int, int);
    int    _mpyhir  (int, int);
    double _mpyli   (int, int);
    int    _mpylir  (int, int);
    double _mpysu4  (int, uint);
    double _mpyu4   (uint, uint);
    uint   _pack2   (uint, uint);
    uint   _packh2  (uint, uint);
    uint   _packh4  (uint, uint);
    uint   _packhl2 (uint, uint);
    uint   _packl4  (uint, uint);
    uint   _packlh2 (uint, uint);
    uint   _rotl    (uint, uint);
    int    _sadd2   (int, int);
    uint   _saddu4  (uint, uint);
    int    _saddus2 (uint, i);
    uint   _shlmb   (uint, uint);
    int    _shr2    (int, uint);
    uint   _shrmb   (uint, uint);
    uint   _shru2   (uint, uint);
    double _smpy2   (int, int);
    int    _spack2  (int, int);
    uint   _spacku4 (int, int);
    int    _sshvl   (int, int);
    int    _sshvr   (int, int);
    int    _sub4    (int, int);
    int    _subabs4 (int, int);
    int    _abs2    (int, int);
    uint   _bitc4   (uint);
    uint   _bitr    (uint);
    uint   _deal    (uint);
    int    _mvd	    (int);
    uint   _shfl    (uint);
    uint   _swap4   (uint);
    uint   _unpkhu4 (uint);
    uint   _unpklu4 (uint);
    uint   _xpnd2   (uint);
    uint   _xpnd4   (uint);
    uint   _cmpyr   (uint, uint);
    uint   _cmpyr1  (uint, uint);
    uint   _gmpy    (uint, uint);
    int    _mpy32   (int, int);
    uint   _rpack2  (uint, uint);
    int    _smpy32  (int, int);
    int    _ssub2   (int, int);
    uint   _xormpy  (uint, uint);

    double       & _memd8	(void*);
    uint         & _mem4	(void*);
    ushort       & _mem2	(void*);
    double       & _amemd8      (void*);
    uint         & _amem4	(void*);
    ushort       & _amem2	(void*);
    const double & _memd8_const	(const void*);
    const uint   & _mem4_const	(const void*);
    const ushort & _mem2_const	(const void*);
    const double & _amemd8_const(const void*);
    const uint   & _amem4_const	(const void*);
    const ushort & _amem2_const	(const void*);


* The OpenCL C builtin function rhadd has been optimized for ucharn and shortn
  data types.


* Changed behavior of local buffers defined in OpenCL C kernel functions to
  conform to the OpenCL spec.  These buffers will now only reserve L2 memory for
  the duration of the kernel.  Previously, they reserved space in L2 from OpenCL
  C program load to program unload.  Any code that may have been written to take
  advantage of this persistent memory, will now be incorrect, as the contents of
  that L2 memory is not guaranteed to persist beyond the current kernel
  execution.


* Any standard C code that is linked with OpenCL C code and also contains
  object definitions with the __attribute__((section(".mem_l2"))) will still
  reserve memory for those object from OpenCL program load to OpenCL program
  unload.  However,  the contents of that memory is not guaranteed to persist
  between kernels calls.  Additionally, any initialization data for those object
  will be ignored
