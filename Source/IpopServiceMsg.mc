;//
;//////////////////////////////////////////////////////////////////////////////

;//  Status values are 32 bit values laid out as follows:
;//
;//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
;//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
;//  +---+-+-------------------------+-------------------------------+
;//  |Sev|C|       Facility          |               Code            |
;//  +---+-+-------------------------+-------------------------------+
;//
;//  where
;//
;//      Sev - is the severity code
;//
;//          00 - Success
;//          01 - Informational
;//          10 - Warning
;//          11 - Error
;//
;//      C - is the Customer code flag
;//
;//      Facility - is the facility code
;//
;//      Code - is the facility's status code
;//
;
SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )
FacilityNames=(IPOPSV=0x2e: FACILITY_IPOPSERVICE)

MessageIdTypedef=DWORD

;//--------------------------------------------------------------------------
;//ERRORS
;//--------------------------------------------------------------------------

MessageId=1 Facility=IPOPSV Severity=Error SymbolicName=IPOP_FAILEDTOSTART
Language=English
The IPop Service failed to acquire resources required to start. 
It cannot continue and will now terminate.
.

MessageId=+1 Facility=IPOPSV Severity=Error SymbolicName=IPOP_MAX_RETRIES
Language=English
The IPoP Service has reached the maximum number of attempts to successfully
start the IPoP processes. No further attempts will be made untill the 
service has been restarted.
.

MessageId=+1 Facility=IPOPSV Severity=Error SymbolicName=IPOP_UNEXPECTED_ERROR
Language=English
The IPoP Service encountered an unexpected error.
.

MessageId=+1 Facility=IPOPSV Severity=Error SymbolicName=IPOP_NO_PYTHON
Language=English
The IPoP Service could not locate the Python 2.7 interpreter. Please ensure
it it is installed and restart the service.
.

MessageId=+1 Facility=IPOPSV Severity=Error SymbolicName=IPOP_NO_TINCAN
Language=English
The IPoP Service could not locate the tincan executable. Please ensure
it it is installed and available in the IPoP installation folder, then
restart the service.
.

MessageId=+1 Facility=IPOPSV Severity=Error SymbolicName=IPOP_NO_CNTRL
Language=English
The IPoP Service could not locate the configured controller. Please ensure
it it is installed and available in the IPoP installation folder, then
restart the service.
.

;//--------------------------------------------------------------------------
;//WARNINGS
;//--------------------------------------------------------------------------
MessageId=+1 Facility=IPOPSV Severity=Warning SymbolicName=IPOP_NOT_OPERATIONAL
Language=English
The IPoP  service was unable to process a service
control since it had not yet completed initialization or was in the processing
of stopping.  If the service is running, retry the command.
.

MessageId=+1 Facility=IPOPSV Severity=Warning SymbolicName=IPOP_RESTART_REQ
LANGUAGE=English
The IPoP processes did not successfully respond to a status check.
An attempt will be made to restart them.
.

;//--------------------------------------------------------------------------
;//INFORMATIONAL
;//--------------------------------------------------------------------------
MessageId=+1 Facility=IPOPSV Severity=Informational SymbolicName=IPOP_START
Language=English
The IPoP Service has been successfully started.
.

MessageId=+1 Facility=IPOPSV Severity=Informational SymbolicName=IPOP_STOPPING
Language=English
The IPoP Service has been requested to stop.
.

MessageId=+1 Facility=IPOPSV Severity=Informational SymbolicName=IPOP_PAUSE
Language=English
The IPoP Service has been paused.
.

MessageId=+1 Facility=IPOPSV Severity=Informational SymbolicName=IPOP_CONTINUE
Language=English
The IPoP Service has successfully resumed operations.
.
