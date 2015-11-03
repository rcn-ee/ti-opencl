******************************************************************
How do I limit log file sizes on EVM's temporary file storage (tmpfs)?
******************************************************************

On TI's EVMs that support OpenCL, log files are usually stored in the
temporary file storage in the volatile memory, which are usually of limited
sizes.  After repeated runs of OpenCL application between EVM reboots, some
log files could potentially use up all available memory.  At that point,
OpenCL application won't be able to run because it also uses /tmp directory
in the same memory.

To avoid the above mentioned problem, one can either reboot the EVM or
redirect some of these logs to "/dev/null" if these logs are not needed
for diagnosis.  The following are the steps to limit some of the most common
log files that could grow too large on various EVMs.

==================
66AK2* EVMs
==================
mpmsrv.log:
  #. Edit /etc/mpm/mpm_config.json
  #. Change "outputif" line to, ``"outputif": "/dev/null",``
  #. Restart mpmsrv or reboot EVM

==================
AM57* EVMs
==================
lad.txt:
  #. Edit /etc/init.d/tiipclad-daemon.sh
  #. Change "-l lad.txt" line to, ``tiipclad_params="-l /dev/null"``
  #. Restart lad_dra7xx or reboot EVM

