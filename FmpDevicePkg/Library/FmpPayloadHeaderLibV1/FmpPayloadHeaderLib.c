/**  @file
  Provides services to retrieve values from Version 1 of a capsule's FMP Payload
  Header. The FMP Payload Header structure is not defined in the library class.
  Instead, services are provided to retrieve information from the FMP Payload
  Header.  If information is added to the FMP Payload Header, then new services
  may be added to this library class to retrieve the new information.

  Copyright (c) 2018, Microsoft Corporation. All rights reserved.<BR>
  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  1. Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#include <PiDxe.h>
#include <Library/FmpPayloadHeaderLib.h>
#include <Library/DebugLib.h>
#include <Library/FmpHelperLib.h>
#include <Library/MemoryAllocationLib.h>

///
/// Define FMP Payload Header structure here so it is not public
///

#pragma pack(1)

typedef struct {
  UINT32  Signature;
  UINT32  HeaderSize;
  UINT32  FwVersion;
  UINT32  LowestSupportedVersion;
  //FW_DEPENDENCY DependencyList[];
} FMP_PAYLOAD_HEADER;

typedef struct {
  EFI_GUID FmpInstance;
  UINT32   RequiredVersionInSystem;
  UINT8    ImageIndex;  //matches the descriptor index
  UINT8    Reserved;
  UINT16   Flags;
} FW_DEPENDENCY;

#pragma pack()

// Flags to describe the expected dependency behaviour 

//dependency must be in system.  Default is only if FMP instance present in system. 
#define FW_DEPENDENCY_FLAG_REQUIRED              0x0001
//version must match exactly.  Default is greater than or equal.
#define FW_DEPENDENCY_FLAG_MATCH_EXACT_VERSION   0x0002

///
/// Identifier is used to make sure the data in the header is for this structure
/// and version.  If the structure changes update the last digit.
///
#define FMP_PAYLOAD_HEADER_SIGNATURE SIGNATURE_32 ('M', 'S', 'S', '1')

/**
  Returns the FMP Payload Header size in bytes.

  @param[in]  Header          FMP Payload Header to evaluate
  @param[in]  FmpPayloadSize  Size of FMP payload
  @param[out] Size            The size, in bytes, of the FMP Payload Header.

  @retval EFI_SUCCESS            The firmware version was returned.
  @retval EFI_INVALID_PARAMETER  Header is NULL.
  @retval EFI_INVALID_PARAMETER  Size is NULL.
  @retval EFI_INVALID_PARAMETER  Header is not a valid FMP Payload Header.

**/
EFI_STATUS
EFIAPI
GetFmpPayloadHeaderSize (
  IN  CONST VOID   *Header,
  IN  CONST UINTN  FmpPayloadSize,
  OUT UINT32       *Size
  )
{
  FMP_PAYLOAD_HEADER  *FmpPayloadHeader;

  FmpPayloadHeader = NULL;

  if (Header == NULL || Size == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FmpPayloadHeader = (FMP_PAYLOAD_HEADER *)Header;
  if ((UINTN)FmpPayloadHeader + sizeof (FMP_PAYLOAD_HEADER) < (UINTN)FmpPayloadHeader ||
      (UINTN)FmpPayloadHeader + sizeof (FMP_PAYLOAD_HEADER) >= (UINTN)FmpPayloadHeader + FmpPayloadSize ||
      FmpPayloadHeader->HeaderSize < sizeof (FMP_PAYLOAD_HEADER)) {
    return EFI_INVALID_PARAMETER;
  }

  if (FmpPayloadHeader->Signature != FMP_PAYLOAD_HEADER_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  *Size = FmpPayloadHeader->HeaderSize;
  return EFI_SUCCESS;
}

/**
  Returns the version described in the FMP Payload Header.

  @param[in]  Header          FMP Payload Header to evaluate
  @param[in]  FmpPayloadSize  Size of FMP payload
  @param[out] Version         The firmware version described in the FMP Payload
                              Header.

  @retval EFI_SUCCESS            The firmware version was returned.
  @retval EFI_INVALID_PARAMETER  Header is NULL.
  @retval EFI_INVALID_PARAMETER  Version is NULL.
  @retval EFI_INVALID_PARAMETER  Header is not a valid FMP Payload Header.

**/
EFI_STATUS
EFIAPI
GetFmpPayloadHeaderVersion (
  IN  CONST VOID   *Header,
  IN  CONST UINTN  FmpPayloadSize,
  OUT UINT32       *Version
  )
{
  FMP_PAYLOAD_HEADER  *FmpPayloadHeader;

  FmpPayloadHeader = NULL;

  if (Header == NULL || Version == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FmpPayloadHeader = (FMP_PAYLOAD_HEADER *)Header;
  if ((UINTN)FmpPayloadHeader + sizeof (FMP_PAYLOAD_HEADER) < (UINTN)FmpPayloadHeader ||
      (UINTN)FmpPayloadHeader + sizeof (FMP_PAYLOAD_HEADER) >= (UINTN)FmpPayloadHeader + FmpPayloadSize ||
      FmpPayloadHeader->HeaderSize < sizeof (FMP_PAYLOAD_HEADER)) {
    return EFI_INVALID_PARAMETER;
  }

  if (FmpPayloadHeader->Signature != FMP_PAYLOAD_HEADER_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  *Version = FmpPayloadHeader->FwVersion;
  return EFI_SUCCESS;
}

/**
  Returns the lowest supported version described in the FMP Payload Header.

  @param[in]  Header                  FMP Payload Header to evaluate
  @param[in]  FmpPayloadSize          Size of FMP payload
  @param[out] LowestSupportedVersion  The lowest supported version described in
                                      the FMP Payload Header.

  @retval EFI_SUCCESS            The lowest support version was returned.
  @retval EFI_INVALID_PARAMETER  Header is NULL.
  @retval EFI_INVALID_PARAMETER  LowestSupportedVersion is NULL.
  @retval EFI_INVALID_PARAMETER  Header is not a valid FMP Payload Header.

**/
EFI_STATUS
EFIAPI
GetFmpPayloadHeaderLowestSupportedVersion (
  IN     CONST VOID   *Header,
  IN     CONST UINTN  FmpPayloadSize,
  IN OUT UINT32       *LowestSupportedVersion
  )
{
  FMP_PAYLOAD_HEADER  *FmpPayloadHeader;

  FmpPayloadHeader = NULL;

  if (Header == NULL || LowestSupportedVersion == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FmpPayloadHeader = (FMP_PAYLOAD_HEADER *)Header;
  if ((UINTN)FmpPayloadHeader + sizeof (FMP_PAYLOAD_HEADER) < (UINTN)FmpPayloadHeader ||
      (UINTN)FmpPayloadHeader + sizeof (FMP_PAYLOAD_HEADER) >= (UINTN)FmpPayloadHeader + FmpPayloadSize ||
      FmpPayloadHeader->HeaderSize < sizeof (FMP_PAYLOAD_HEADER)) {
    return EFI_INVALID_PARAMETER;
  }

  if (FmpPayloadHeader->Signature != FMP_PAYLOAD_HEADER_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  *LowestSupportedVersion = FmpPayloadHeader->LowestSupportedVersion;
  return EFI_SUCCESS;
}

/**
function that evaluated all dependencies listed in the FmpHeader
and sets the Verified value to the TRUE if all are successfully met.

@param  Header    Header to evaluate
@param  Verified  The result of checking dependencies

@return  EFI_SUCCESS on success.
@return  EFI_INVALID_PARAMETER if Header is not a valid Fmp Header

**/
EFI_STATUS
EFIAPI
VerifyFmpPayloadDependencies(
  IN       CONST VOID*     Header,
  IN OUT   BOOLEAN*        Verified
  )
{
  FMP_PAYLOAD_HEADER             *FmpPayloadHeader;
  EFI_FIRMWARE_IMAGE_DESCRIPTOR  *FmpDesc;
  FW_DEPENDENCY                  *Dep;
  INTN                           DependencyBytes;
  INTN                           NumberOfDeps
  EFI_STATUS                     Status;

  FmpPayloadHeader = NULL;
  FmpDesc          = NULL;
  Dep              = NULL;
  DependencyBytes  = 0;
  NumberOfDeps     = 0;
  Status           = EFI_SUCCESS;

  if (Header == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }

  FmpPayloadHeader = (FMP_PAYLOAD_HEADER*)Header;

  if (FmpPayloadHeader->Identifier != FMP_PAYLOAD_HEADER_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  DependencyBytes = (INTN)FmpPayloadHeader->HeaderSize - sizeof(FMP_PAYLOAD_HEADER);
  DEBUG((DEBUG_INFO, "FmpPayload: %a - # of dependency bytes 0x%X\n", __FUNCTION__, DependencyBytes));

  if (DependencyBytes > 0)
  {
    NumberOfDeps = DependencyBytes / sizeof(FW_DEPENDENCY);
    if ((DependencyBytes % sizeof(FW_DEPENDENCY)) != 0) {
      //Dependency list has an invalid size
      DEBUG((DEBUG_ERROR, "FmpPayload: %a - Dependency section of header is invalid size. 0x%X\n", __FUNCTION__, DependencyBytes));
      *Verified = FALSE;
      Status =  EFI_INVALID_PARAMETER;
      goto EXIT;
    }
    DEBUG((DEBUG_INFO, "FmpPayload: %a - Processing %d dependencies\n", __FUNCTION__, NumberOfDeps));
    
    //Loop through all dependencies
    Dep = (FW_DEPENDENCY *)(((UINT8 *)FmpPayloadHeader) + sizeof(FMP_PAYLOAD_HEADER));
    while (NumberOfDeps > 0)
    {
      Status = GetFmpAndDescriptor(&Dep->FmpInstance, Dep->ImageIndex, NULL, &FmpDesc, NULL);
      if (EFI_ERROR(Status))
      {
        DEBUG((DEBUG_WARN, "FmpPayload: %a - GetFmpAndDescriptor failed for dependency. Status %r\n", __FUNCTION__, Status));
        //
        // error occurred
        // fail if required flag set
        //
        if ((Dep->Flags & FW_DEPENDENCY_FLAG_REQUIRED) == FW_DEPENDENCY_FLAG_REQUIRED)
        {
          DEBUG((DEBUG_ERROR, "FmpPayload: Dependency for FMP %g failed. Can not do capsule update. \n", &Dep->FmpInstance));
          *Verified = FALSE;
          Status = EFI_SUCCESS;
          goto EXIT;
        }
      }
      else
      {
        //
        // if sys version is less than dep min required version  - fail
        //
        if (Dep->RequiredVersionInSystem > FmpDesc->Version)
        {
          DEBUG((DEBUG_ERROR, "FmpPayload: Dependency for FMP %g failed. Version on system (0x%X) is older than required (0x%X). \n", &Dep->FmpInstance, FmpDesc->Version, Dep->RequiredVersionInSystem));
          *Verified = FALSE;
          Status = EFI_SUCCESS;
          goto EXIT;
        }

        //
        // If flag for exact match and not exact match - fail
        //
        if (((Dep->Flags & FW_DEPENDENCY_FLAG_MATCH_EXACT_VERSION) == FW_DEPENDENCY_FLAG_MATCH_EXACT_VERSION) && (Dep->RequiredVersionInSystem != FmpDesc->Version) )
        {
          DEBUG((DEBUG_ERROR, "FmpPayload: Dependency for FMP %g failed. Version on system (0x%X) is not the exact required (0x%X). \n", &Dep->FmpInstance, FmpDesc->Version, Dep->RequiredVersionInSystem));
          *Verified = FALSE;
          Status = EFI_SUCCESS;
          goto EXIT;
        }

        //Check for unknown flags
        if(((FW_DEPENDENCY_FLAG_MATCH_EXACT_VERSION | FW_DEPENDENCY_FLAG_REQUIRED) & Dep->Flags) != Dep->Flags)
        {
          DEBUG((DEBUG_WARN, "FmpPayload: Unknown dependency flag for FMP %g. Dependency flags provided 0x%X\n", &Dep->FmpInstance, Dep->Flags));
        }

        //Dep verified
        DEBUG((DEBUG_INFO, "FmpPayload: Dependency for FMP %g Passed. Version on system (0x%X) meets the requirements defined in header (0x%X). \n", &Dep->FmpInstance, FmpDesc->Version, Dep->RequiredVersionInSystem));
        Status = EFI_SUCCESS;
      }
      NumberOfDeps--;
      Dep++;  //next dep
    }  //end while loop going through each dependency
  }  //if dependency bytes greater than 0

  *Verified = TRUE;

EXIT:
  if (FmpDesc != NULL)
  {
    FreePool(FmpDesc);
  }
  return Status;
}