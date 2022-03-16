#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <ntddscsi.h>

#pragma comment(lib, "wbemuuid.lib")


class HardwareId {

public:


	static std::wstring SafeString(const wchar_t* pString) {
		return std::wstring((pString == nullptr ? L"(null)" : pString));
	}


	static void RemoveWhitespaces(std::wstring& String) {
		String.erase(std::remove(String.begin(), String.end(), L' '), String.end());
	}


private:


	std::wstring GetHKLM(std::wstring SubKey, std::wstring Value) {
		DWORD Size{};
		std::wstring Ret{};

		RegGetValueW(HKEY_LOCAL_MACHINE,
			     SubKey.c_str(),
			     Value.c_str(),
			     RRF_RT_REG_SZ,
			     nullptr,
			     nullptr,
			     &Size);

		Ret.resize(Size);

		RegGetValueW(HKEY_LOCAL_MACHINE,
			     SubKey.c_str(),
			     Value.c_str(),
			     RRF_RT_REG_SZ,
			     nullptr,
			     &Ret[0],
			     &Size);

		return Ret.c_str();
	}


	template <typename T = const wchar_t*>
	void QueryWMI(std::wstring WMIClass, std::wstring Field, std::vector <T>& Value, const wchar_t* ServerName = L"ROOT\\CIMV2") {
		std::wstring Query(L"SELECT ");
		Query.append(Field.c_str()).append(L" FROM ").append(WMIClass.c_str());

		IWbemLocator* Locator{};
		IWbemServices* Services{};
		IEnumWbemClassObject* Enumerator{};
		IWbemClassObject* ClassObject{};
		VARIANT Variant{};
		DWORD Returned{};


		HRESULT hResult{ CoInitializeEx(nullptr, COINIT_MULTITHREADED) };


		if (FAILED(hResult)) {
			return;
		}


		hResult = CoInitializeSecurity(nullptr,
			                       -1,
					       nullptr,
					       nullptr,
					       RPC_C_AUTHN_LEVEL_DEFAULT,
					       RPC_C_IMP_LEVEL_IMPERSONATE,
					       nullptr,
					       EOAC_NONE,
					       nullptr);



		if (FAILED(hResult)) {
			CoUninitialize();
			return;
		}


		hResult = CoCreateInstance(CLSID_WbemLocator,
					   NULL,
					   CLSCTX_INPROC_SERVER,
					   IID_IWbemLocator,
					   reinterpret_cast<PVOID*>(&Locator));


		if (FAILED(hResult)) {
			CoUninitialize();
			return;
		}


		hResult = Locator->ConnectServer(_bstr_t(ServerName),
						 nullptr,
						 nullptr,
						 nullptr,
						 NULL,
						 nullptr,
						 nullptr,
						 &Services);


		if (FAILED(hResult)) {
			Locator->Release();
			CoUninitialize();
			return;
		}


		hResult = CoSetProxyBlanket(Services,
					    RPC_C_AUTHN_WINNT,
					    RPC_C_AUTHZ_NONE,
				  	    nullptr,
					    RPC_C_AUTHN_LEVEL_CALL,
					    RPC_C_IMP_LEVEL_IMPERSONATE,
					    nullptr,
					    EOAC_NONE);


		if (FAILED(hResult)) {
			Services->Release();
			Locator->Release();
			CoUninitialize();
			return;
		}


		hResult = Services->ExecQuery(bstr_t(L"WQL"),
					      bstr_t(Query.c_str()),
					      WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
					      nullptr,
					      &Enumerator);


		if (FAILED(hResult)) {
			Services->Release();
			Locator->Release();
			CoUninitialize();
			return;
		}

		while (Enumerator) {

			HRESULT Res = Enumerator->Next(WBEM_INFINITE,
						       1,
						       &ClassObject,
						       &Returned);

			if (!Returned) {
				break;
			}

			Res = ClassObject->Get(Field.c_str(), 
					       0, 
					       &Variant, 
					       nullptr, 
					       nullptr);

			if (typeid(T) == typeid(long) || typeid(T) == typeid(int)) {
				Value.push_back((T)Variant.intVal);
			}
			else if (typeid(T) == typeid(bool)) {
				Value.push_back((T)Variant.boolVal);
			}
			else if (typeid(T) == typeid(unsigned int)) {
				Value.push_back((T)Variant.uintVal);
			}
			else if (typeid(T) == typeid(unsigned short)) {
				Value.push_back((T)Variant.uiVal);
			}
			else if (typeid(T) == typeid(long long)) {
				Value.push_back((T)Variant.llVal);
			}
			else if (typeid(T) == typeid(unsigned long long)) {
				Value.push_back((T)Variant.ullVal);
			}
			else {
				Value.push_back((T)((bstr_t)Variant.bstrVal).copy());
			}

			VariantClear(&Variant);
			ClassObject->Release();
		}

		if (!Value.size()) {
			Value.resize(1);
		}

		Services->Release();
		Locator->Release();
		Enumerator->Release();
		CoUninitialize();
	}


	void QueryDisk() {

		std::wstring DrivePath{ L"\\\\.\\PhysicalDrive" };
		std::wstring VolumePath{ L"\\\\.\\" };
		HANDLE hVolume{ nullptr };
		VOLUME_DISK_EXTENTS DiskExtents{ NULL };
		DWORD IoBytes{ NULL };
		ULARGE_INTEGER FreeBytesAvailable{};
		ULARGE_INTEGER TotalBytes{};
		int DriveCount{ 0 };


		std::vector <const wchar_t*> SerialNumber{};
		std::vector <const wchar_t*> Model{};
		std::vector <const wchar_t*> Interface{};
		std::vector <const wchar_t*> Name{};
		std::vector <const wchar_t*> DeviceId{};
		std::vector <const wchar_t*> SortedDeviceId{};
		std::vector <const wchar_t*> FriendlyName{};
		std::vector <unsigned int> MediaType{};
		std::vector <bool> IsBoot{};


		// We need a count of the actual physical disks active on the system
		// The simplest way is to attempt to open a handle to PhysicalDriveX (where X is 0, 1, 2, etc.)
		// If we can get a valid handle, we increment the drive count, else we break

		HANDLE Handle{ nullptr };

		for (;; DriveCount++) {
			if ((Handle = CreateFileW((DrivePath + std::to_wstring(DriveCount)).c_str(), 
						   NULL, 
						   NULL,
						   nullptr,
						   OPEN_EXISTING, 
						   NULL,
						   nullptr)) == INVALID_HANDLE_VALUE) { break; }

			CloseHandle(Handle);
		}


		this->Disk.resize(DriveCount);
		SortedDeviceId.resize(DriveCount);


		// To get most of the data we want, we make several queries to the Windows Management Instrumentation (WMI) service
		// Queries to MSFT_PhysicalDisk and MSFT_Disk require a connection to the ROOT\\microsoft\\windows\\storage namespace

		QueryWMI(L"Win32_DiskDrive", L"SerialNumber", SerialNumber);
		QueryWMI(L"Win32_DiskDrive", L"Model", Model);
		QueryWMI(L"Win32_DiskDrive", L"InterfaceType", Interface);
		QueryWMI(L"Win32_DiskDrive", L"Name", Name);
		QueryWMI(L"Win32_LogicalDisk", L"DeviceId", DeviceId);
		QueryWMI(L"MSFT_PhysicalDisk", L"MediaType", MediaType, L"ROOT\\microsoft\\windows\\storage");
		QueryWMI(L"MSFT_PhysicalDisk", L"FriendlyName", FriendlyName, L"ROOT\\microsoft\\windows\\storage");
		QueryWMI(L"MSFT_Disk", L"IsBoot", IsBoot, L"ROOT\\microsoft\\windows\\storage");


		for (int i = 0; i < DriveCount; i++) {
			for (int j = 0; j < DriveCount; j++) {

				// Win32_LogicalDisk is relied on to get the drive letter (DeviceId)
				// However, the data it returns will not be in the same order as the results we get from Win32_DiskDrive
				// The drive letter is what we rely on to get the total size and free space of the drive, so we must map the data accordingly
				// Win32_DiskDrive will return data in the order we want. It'll always be PhysicalDrive0, then PhysicalDrive1, then 2, etc.

				// Armed with that knowledge, we can open handles directly to the drive letters we have, and use DeviceIoControl to call into the volume

				hVolume = CreateFileW((VolumePath + DeviceId.at(j)).c_str(),
						      NULL,
						      NULL,
						      nullptr,
						      OPEN_EXISTING,
						      NULL,
						      nullptr);


				// IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS will fill our buffer with a VOLUME_DISK_EXTENTS structure
				// First, we must get the number of disk extents

				DeviceIoControl(hVolume,
						IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
						nullptr,
						NULL,
						&DiskExtents,
						sizeof(DiskExtents),
						&IoBytes,
						nullptr);


				// VOLUME_DISK_EXTENTS contains an array of DISK_EXTENT structures. DISK_EXTENT contains a DWORD member, DiskNumber
				// DiskNumber will be the same number used to construct the name of the disk, which is PhysicalDriveX, where X is the DiskNumber

				DeviceIoControl(hVolume,
						IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
						nullptr,
						NULL,
						&DiskExtents,
						offsetof(VOLUME_DISK_EXTENTS, Extents[DiskExtents.NumberOfDiskExtents]),
						&IoBytes,
						nullptr);

				CloseHandle(hVolume);


				// To map the drive letter from Win32_LogicalDisk to the data returned by Win32_DiskDrive
				// We compare the drive letter's DiskNumber to the number at the end of the "Name" we recieve from Win32_DiskDrive
				// We then reorder the drive letters accordingly

				if (DiskExtents.Extents->DiskNumber == std::stoi(&SafeString(Name.at(i)).back())) {
					SortedDeviceId.at(i) = DeviceId.at(j);
					break;
				}
			}

		}


		this->Disk.resize(DriveCount);


		for (int i = 0; i < DriveCount; i++) {

			// GetDiskFreeSpaceEx() will give us the size and free space available corresponding to the drive letters we have

			GetDiskFreeSpaceEx(SafeString(SortedDeviceId.at(i)).c_str(),
					   &FreeBytesAvailable,
					   &TotalBytes,
					   nullptr);

			RemoveWhitespaces(this->Disk.at(i).SerialNumber = SafeString(SerialNumber.at(i)));
			this->Disk.at(i).Model = SafeString(Model.at(i));
			this->Disk.at(i).Interface = SafeString(Interface.at(i));
			this->Disk.at(i).DriveLetter = SafeString(SortedDeviceId.at(i));
			this->Disk.at(i).Size = TotalBytes.QuadPart / pow(1024, 3);
			this->Disk.at(i).FreeSpace = FreeBytesAvailable.QuadPart / pow(1024, 3);
			this->Disk.at(i).IsBootDrive = IsBoot.at(i);


			// Data from MSFT_PhysicalDisk will not be in the same order as Win32_DiskDrive
			// So we compare the "FriendlyName" from MSFT_PhysicalDisk with the "Model" from Win32_DiskDrive
			// We then reorder the data accordingly

			for (int j = 0; j < DriveCount; j++) {
				if (!this->Disk.at(i).Model.compare(FriendlyName.at(j))) {
					this->Disk.at(i).MediaType = MediaType.at(j);
				}
			}
		}
	}


	void QuerySMBIOS() {
		std::vector <const wchar_t*> Manufacturer{};
		std::vector <const wchar_t*> Product{};
		std::vector <const wchar_t*> Version{};
		std::vector <const wchar_t*> SerialNumber{};

		QueryWMI(L"Win32_BaseBoard", L"Manufacturer", Manufacturer);
		QueryWMI(L"Win32_BaseBoard", L"Product", Product);
		QueryWMI(L"Win32_BaseBoard", L"Version", Version);
		QueryWMI(L"Win32_BaseBoard", L"SerialNumber", SerialNumber);


		this->SMBIOS.Manufacturer = SafeString(Manufacturer.at(0));
		this->SMBIOS.Product = SafeString(Product.at(0));
		this->SMBIOS.Version = SafeString(Version.at(0));
		this->SMBIOS.SerialNumber = SafeString(SerialNumber.at(0));
	}


	void QueryProcessor() {
		std::vector <const wchar_t*> ProcessorId{};
		std::vector <const wchar_t*> Manufacturer{};
		std::vector <const wchar_t*> Name{};
		std::vector <int> Cores{};
		std::vector <int> Threads{};

		QueryWMI(L"Win32_Processor", L"ProcessorId", ProcessorId);
		QueryWMI(L"Win32_Processor", L"Manufacturer", Manufacturer);
		QueryWMI(L"Win32_Processor", L"Name", Name);
		QueryWMI<int>(L"Win32_Processor", L"NumberOfCores", Cores);
		QueryWMI<int>(L"Win32_Processor", L"NumberOfLogicalProcessors", Threads);

		this->CPU.ProcessorId = SafeString(ProcessorId.at(0));
		this->CPU.Manufacturer = SafeString(Manufacturer.at(0));
		this->CPU.Name = SafeString(Name.at(0));
		this->CPU.Cores = Cores.at(0);
		this->CPU.Threads = Threads.at(0);
	}


	void QueryGPU() {
		std::vector <const wchar_t*> Name{};
		std::vector <const wchar_t*> DriverVersion{};
		std::vector <unsigned long long> Memory{};
		std::vector <int> XRes{};
		std::vector <int> YRes{};
		std::vector <int> RefreshRate{};

		QueryWMI(L"Win32_VideoController", L"Name", Name);
		QueryWMI(L"Win32_VideoController", L"DriverVersion", DriverVersion);
		QueryWMI(L"Win32_VideoController", L"AdapterRam", Memory);
		QueryWMI(L"Win32_VideoController", L"CurrentHorizontalResolution", XRes);
		QueryWMI(L"Win32_VideoController", L"CurrentVerticalResolution", YRes);
		QueryWMI(L"Win32_VideoController", L"CurrentRefreshRate", RefreshRate);

		this->GPU.resize(Name.size());

		for (int i = 0; i < Name.size(); i++) {
			this->GPU.at(i).Name = SafeString(Name.at(i));
			this->GPU.at(i).DriverVersion = SafeString(DriverVersion.at(i));
			this->GPU.at(i).Memory = (Memory.at(i) * 2 / ( 1024 * 1024 ) / 1000);
			this->GPU.at(i).XResolution = XRes.at(i);
			this->GPU.at(i).YResolution = YRes.at(i);
			this->GPU.at(i).RefreshRate = RefreshRate.at(i);
		}
	}


	void QuerySystem() {
		std::vector <const wchar_t*> SystemName{};
		std::vector <const wchar_t*> OSVersion{};
		std::vector <const wchar_t*> OSName{};
		std::vector <const wchar_t*> OSArchitecture{};
		std::vector <const wchar_t*> OSSerialNumber{};
		std::vector <bool> IsHypervisorPresent{};

		QueryWMI(L"Win32_ComputerSystem", L"Name", SystemName);
		QueryWMI(L"Win32_ComputerSystem", L"Model", IsHypervisorPresent);
		
		QueryWMI(L"Win32_OperatingSystem", L"Version", OSVersion);
		QueryWMI(L"Win32_OperatingSystem", L"Name", OSName);
		QueryWMI(L"Win32_OperatingSystem", L"OSArchitecture", OSArchitecture);
		QueryWMI(L"Win32_OperatingSystem", L"SerialNumber", OSSerialNumber);

		std::wstring wOSName{ SafeString(OSName.at(0)) };

		if (wOSName.find('|') != std::wstring::npos) {
			wOSName.resize(wOSName.find('|'));
		}

		this->System.Name = SafeString(SystemName.at(0));
		this->System.IsHypervisorPresent = IsHypervisorPresent.at(0);
		this->System.OSName = wOSName;
		this->System.OSVersion = SafeString(OSVersion.at(0));
		this->System.OSSerialNumber = SafeString(OSSerialNumber.at(0));
		this->System.OSArchitecture = SafeString(OSArchitecture.at(0));
	}


	void QueryNetwork() {
		std::vector <const wchar_t*> Name{};
		std::vector <const wchar_t*> MAC{};

		QueryWMI(L"Win32_NetworkAdapter", L"Name", Name);
		QueryWMI(L"Win32_NetworkAdapter", L"MACAddress", MAC);


		this->NetworkAdapter.resize(Name.size());

		for (int i = 0; i < Name.size(); i++) {
			this->NetworkAdapter.at(i).Name = SafeString(Name.at(i));
			this->NetworkAdapter.at(i).MAC = SafeString(MAC.at(i));
		}
	}


	void QueryPhysicalMemory() {
		std::vector<const wchar_t*> PartNumber{};

		QueryWMI(L"Win32_PhysicalMemory", L"PartNumber", PartNumber);
		this->PhysicalMemory.PartNumber = SafeString(PartNumber.at(0));
	}


	void QueryRegistry() {
		this->Registry.ComputerHardwareId = SafeString(GetHKLM(L"SYSTEM\\CurrentControlSet\\Control\\SystemInformation", L"ComputerHardwareId").c_str());
	}


	void GetHardwareId() {
		QueryDisk();
		QuerySMBIOS();
		QueryProcessor();
		QueryGPU();
		QuerySystem();
		QueryNetwork();
		QueryPhysicalMemory();
		QueryRegistry();
	}


public:


	struct DiskObject {

		std::wstring SerialNumber{};
		std::wstring Model{};		 // e.g. ADATA SU740
		std::wstring Interface{};	 // e.g. SCSI
		std::wstring DriveLetter{};
		long long Size{};		 // In GB
		long long FreeSpace{};		 // In GB
		unsigned int MediaType{};	 // 4 = SSD, 3 = HDD, 0 = Unspecified, 5 = SCM
		bool IsBootDrive{};

	}; std::vector <DiskObject> Disk{};


	struct {

		std::wstring Manufacturer{};
		std::wstring Product{};
		std::wstring Version{};
		std::wstring SerialNumber{};

	} SMBIOS;


	struct {

		std::wstring ProcessorId{};
		std::wstring Manufacturer{};
		std::wstring Name{};
		int Cores{};
		int Threads{};

	} CPU;


	struct GPUObject {

		std::wstring Name{};
		std::wstring DriverVersion{};
		unsigned long long Memory{};
		int XResolution{};
		int YResolution{};
		int RefreshRate{};

	}; std::vector <GPUObject> GPU{};


	struct NetworkAdapterObject {

		std::wstring Name{};
		std::wstring MAC{};

	}; std::vector <NetworkAdapterObject> NetworkAdapter{};



	struct {

		std::wstring Name{};
		bool IsHypervisorPresent{};
		std::wstring OSVersion{};
		std::wstring OSName{};
		std::wstring OSArchitecture{};
		std::wstring OSSerialNumber{};

	} System;


	struct {

		std::wstring PartNumber{};

	} PhysicalMemory;


	struct {

		std::wstring ComputerHardwareId{};

	} Registry;


	std::unique_ptr<HardwareId> Pointer() {
		return std::make_unique<HardwareId>(*this);
	}


	HardwareId() {
		GetHardwareId();
	}
};
