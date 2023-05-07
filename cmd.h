#pragma once

#include <algorithm>
#include "hwid.h"


namespace CommandLine {

	HardwareId HWID{};
	std::wstring CurCmd{ L"" };
	std::wstring CmdName{ L"HWInfo>" };


	enum {
		eInvalid,
		eDisk,
		eSMBIOS,
		eGPU,
		eCPU,
		eNetwork,
		eSystem,
		ePhysicalMemory,
		eRegistry,
		eAll,
		eHelp,
		eExit
	};


	std::unordered_map<unsigned int, std::wstring> BusType {
		{ 0, L"Unspecified" },
		{ 1, L"SCSI" },
		{ 3, L"ATA" },
		{ 7, L"USB" },
		{ 8, L"RAID" },
		{ 11, L"SATA" },
		{ 17, L"NVMe" },
		{ 123, L"USB (Likely Flash Drive)" }
	};


	std::vector <std::wstring> ValidCommands{
		L"invalid command",
		L"disk",
		L"smbios",
		L"gpu",
		L"cpu",
		L"network",
		L"system",
		L"physicalmemory",
		L"registry",
		L"all",
		L"help",
		L"exit"
	};


	std::vector <std::wstring> DiskSubCommands{
		L"invalid command",
		L"serialnumber",
		L"model",
		L"interface",
		L"driveletter",
		L"size",
		L"freespace",
		L"mediatype",
		L"isbootdrive",
		L"bustype",
		L"volumes"
	};


	std::vector <std::wstring> SMBIOSSubCommands{
		L"invalid command",
		L"serialnumber",
		L"manufacturer",
		L"product",
		L"version"
	};


	std::vector <std::wstring> GPUSubCommands{
		L"invalid command",
		L"name",
		L"driverversion",
		L"resolution",
		L"refreshrate",
		L"memory"
	};


	std::vector <std::wstring> CPUSubCommands{
		L"invalid command",
		L"processorid",
		L"manufacturer",
		L"name",
		L"cores",
		L"threads"
	};


	std::vector <std::wstring> NetworkSubCommands{
		L"invalid command",
		L"name",
		L"mac"
	};


	std::vector <std::wstring> SystemSubCommands{
		L"invalid command",
		L"name",
		L"ishypervisorpresent",
		L"osversion",
		L"ostitle",
		L"osarchitecture",
		L"osserialnumber"
	};


	std::vector <std::wstring> PhysicalMemorySubCommands{
		L"invalid command",
		L"partnumber"
	};


	std::vector <std::wstring> RegistrySubCommands{
		L"invalid command",
		L"computerhardwareid"
	};


	struct {

		int CommandIndex{};
		std::vector <int> SubCommandIndex{};

	} ParsedCommand;


	void ChangeTextColor(int Color = 2) {
		HANDLE hConsole{ GetStdHandle(STD_OUTPUT_HANDLE) };
		SetConsoleTextAttribute(hConsole, Color);
	}


	template <typename T>
	void FmtPrint(T ToPrint, int HWType, int Iterator, const wchar_t* AdditionalText = nullptr) {
		std::wcout << ToPrint;
		
		if (AdditionalText != nullptr) {
			std::wcout << AdditionalText;
		}
		

		switch (HWType) {
			case eDisk: {
				std::wcout << (Iterator && Iterator + 1 == HWID.Disk.size() ? L"\n\n" : L"\n");
			} break;

			case eGPU: {
				std::wcout << (Iterator && Iterator + 1 == HWID.GPU.size() ? L"\n\n" : L"\n");
			} break;

			case eNetwork: {
				std::wcout << (Iterator && Iterator + 1 == HWID.NetworkAdapter.size() ? L"\n\n" : L"\n");
			} break;
		}
	}


	void PrintDisks() {
		for (int i = 0; i < HWID.Disk.size(); i++) {
			HardwareId::DiskObject& Disk{ HWID.Disk.at(i) };

			std::wcout << L"Drive: ";
			ChangeTextColor(12);
			std::wcout << Disk.Name.substr(4, Disk.Name.size() - 4);
			ChangeTextColor(15);
			std::wcout << L"\n---------------------\n";

			std::wcout << L"Model:\t\t\t" << Disk.Model << std::endl;
			std::wcout << L"Serial Number:\t\t" << Disk.SerialNumber << std::endl;
			std::wcout << L"Interface Type:\t\t" << Disk.Interface << std::endl;
			std::wcout << L"Bus Type:\t\t" << BusType[HWID.Disk.at(i).BusType] << std::endl;
			std::wcout << L"Size:\t\t\t" << Disk.Size << L" GB" << std::endl;
			std::wcout << L"Free Space:\t\t" << Disk.FreeSpace << L" GB" << std::endl;

			std::wcout << L"Media Type:\t\t" <<
				(HWID.Disk.at(i).MediaType == 4 ? L"SSD" :
					(HWID.Disk.at(i).MediaType == 3 ? L"HDD" : L"(null)")) << std::endl;

			std::wcout << L"Boot Drive:\t\t" << (Disk.IsBootDrive ? L"Yes" : L"No") << std::endl;

			if (Disk.Volumes.empty() == false) {
				for (int j = 0; j < Disk.Volumes.size(); j++) {
					std::wcout << std::endl;
					std::wcout << L"Volume " << j << std::endl << L"--------\n";
					std::wcout << L"Name:\t\t\t" << Disk.Volumes.at(j).Name << std::endl;
					std::wcout << L"Serial Number:\t\t" << std::hex << Disk.Volumes.at(j).SerialNumber << std::dec << std::endl;
					std::wcout << L"Size:\t\t\t" << Disk.Volumes.at(j).Size << L" GB" << std::endl;
					std::wcout << L"Free Space:\t\t" << Disk.Volumes.at(j).FreeSpace << L" GB" << std::endl;
					std::wcout << L"Drive Letter:\t\t" << Disk.Volumes.at(j).DriveLetter << std::endl;

					if (j + 1 == Disk.Volumes.size()) {
						std::wcout << std::endl;
					}
				}
			}
			else {
				std::wcout << L"Drive Letter:\t\t" << HWID.Disk.at(i).DriveLetter << std::endl;
				
				if (i + 1 != HWID.Disk.size()) {
					std::wcout << std::endl;
				}
			}
		}
	}


	void PrintSMBIOS() {
		std::wcout << L"Manufacturer:\t\t" << HWID.SMBIOS.Manufacturer << std::endl;
		std::wcout << L"Product:\t\t" << HWID.SMBIOS.Product << std::endl;
		std::wcout << L"Version:\t\t" << HWID.SMBIOS.Version << std::endl;
		std::wcout << L"Serial Number:\t\t" << HWID.SMBIOS.SerialNumber << std::endl;
	}


	void PrintGPUs() {
		for (int i = 0; i < HWID.GPU.size(); i++) {
			std::wcout << L"Name:\t\t\t" << HWID.GPU.at(i).Name << std::endl;
			std::wcout << L"Driver Version:\t\t" << HWID.GPU.at(i).DriverVersion << std::endl;
			std::wcout << L"Memory:\t\t\t" << HWID.GPU.at(i).Memory << L" MB" << std::endl;
			std::wcout << L"Resolution:\t\t" << HWID.GPU.at(i).XResolution << L"x" << HWID.GPU.at(i).YResolution << std::endl;
			std::wcout << L"Refresh Rate:\t\t" << HWID.GPU.at(i).RefreshRate << std::endl;

			if (i + 1 < HWID.GPU.size()) { std::wcout << std::endl; }
		}
	}


	void PrintCPU() {
		std::wcout << L"Processor Id:\t\t" << HWID.CPU.ProcessorId << std::endl;
		std::wcout << L"Manufacturer:\t\t" << HWID.CPU.Manufacturer << std::endl;
		std::wcout << L"Name:\t\t\t" << HWID.CPU.Name << std::endl;
		std::wcout << L"Cores:\t\t\t" << HWID.CPU.Cores << std::endl;
		std::wcout << L"Threads:\t\t" << HWID.CPU.Threads << std::endl;
	}


	void PrintNetwork() {
		for (int i = 0; i < HWID.NetworkAdapter.size(); i++) {
			std::wcout << L"Name:\t\t\t" << HWID.NetworkAdapter.at(i).Name << std::endl;
			std::wcout << L"MAC Address:\t\t" << HWID.NetworkAdapter.at(i).MAC << std::endl;

			if (i + 1 < HWID.NetworkAdapter.size()) { std::wcout << std::endl; }
		}
	}


	void PrintSystem() {
		std::wcout << L"System Name:\t\t" << HWID.System.Name << std::endl;
		std::wcout << L"Hypervisor Present:\t" << (HWID.System.IsHypervisorPresent ? L"Yes" : L"No") << std::endl;
		std::wcout << L"OS Title:\t\t" << HWID.System.OSName << std::endl;
		std::wcout << L"OS Version:\t\t" << HWID.System.OSVersion << std::endl;
		std::wcout << L"OS Architecture:\t" << HWID.System.OSArchitecture << std::endl;
		std::wcout << L"OS Serial Number:\t" << HWID.System.OSSerialNumber << std::endl;
	}


	void PrintPhysicalMemory() {
		std::wcout << L"Part Number:\t\t" << HWID.PhysicalMemory.PartNumber << std::endl;
	}


	void PrintRegistry() {
		std::wcout << L"Computer Hardware Id:\t" << HWID.Registry.ComputerHardwareId << std::endl;
	}


	std::vector <std::wstring> ValidSubCommands() {
		switch (ParsedCommand.CommandIndex) {
			case eDisk: {
				return DiskSubCommands;
			} break;

			case eSMBIOS: {
				return SMBIOSSubCommands;
			} break;

			case eGPU: {
				return GPUSubCommands;
			} break;

			case eCPU: {
				return CPUSubCommands;
			} break;

			case eNetwork: {
				return NetworkSubCommands;
			} break;

			case eSystem: {
				return SystemSubCommands;
			} break;

			case ePhysicalMemory: {
				return PhysicalMemorySubCommands;
			} break;

			case eRegistry: {
				return RegistrySubCommands;
			} break;
		}

		return {};
	}


	void ParseCommand() {
		bool bFound{ false };

		auto OrderVector{ [] (std::vector <int> ToOrder) -> std::vector <int> {
			struct MinToMax {
				MinToMax(const std::vector <int> &Vector) : _Vector(Vector) {}

				bool operator () (int Index, int Index2) {
					return _Vector[Index] < _Vector[Index2];
				}

				const std::vector <int>& _Vector{};
			};

			std::vector <int> OrderedElements{};
			OrderedElements.resize(ToOrder.size());

			for (int i = 0; i < ToOrder.size(); ++i) {
				OrderedElements.at(i) = i;
			}

			std::sort(OrderedElements.begin(), OrderedElements.end(), MinToMax(ToOrder));
			return OrderedElements;
		} };


		// Convert the command to lowercase to avoid any case issues

		for (int i = 0; i < CurCmd.size(); i++) {
			CurCmd.at(i) = std::tolower(CurCmd.at(i));
		}


		// example of a valid command: "disk get serialnumber, model"
		// "disk" is the main command
		// "get" is the keyword required to request fields/members
		// "serialnumber" and "model" are sub-commands containing the requested fields


		// Determine if the start of the command contains a valid main command by looping through our vector of valid commands

		for (; ParsedCommand.CommandIndex < ValidCommands.size(); ParsedCommand.CommandIndex++) {
			if (CurCmd.find(ValidCommands.at(ParsedCommand.CommandIndex)) == 0) {
				bFound = true;
				break;
			}
		}


		if (!bFound) {
			// The command is invalid
			ParsedCommand.CommandIndex = eInvalid;
			return;
		}


		// Determine if the command contains a "get" request by checking for the "get" keyword
		// Additionally, make sure the command isn't just "[main command] get" by checking 
		// if it's larger than the size of the main command + the size of "get"

		if (CurCmd.find(L"get") != std::wstring::npos && CurCmd.size() != ValidCommands.at(ParsedCommand.CommandIndex).size() + 3) {

			// Determine if "get" is directly after the main command. If it's not, it's an invalid command 
			// e.g. "disk get serialnumber" is valid, "get disk serialnumber" is not

			if (CurCmd.find(L"get") != ValidCommands.at(ParsedCommand.CommandIndex).size()) {
				ParsedCommand.CommandIndex = eInvalid;
				return;
			}


			// Check if we have valid sub-commands 
			// If we do, we construct our vector of sub-command indices; however, they won't be in the same order as the command request
			// If the full command is "disk get model, serialnumber", the requested fields will be returned in the order of our sub-command vector
			// In this example, the disk serial number(s) will be returned before the model(s) because serial number is before model in our vector
			// even though they specified the model before the serial number.
			// So we must keep track of the starting offset of the sub-commands by storing each one in an additional vector
			// The smallest offsets are closest to the "get" keyword in the command, 
			// so we'll use the indices of them to sort our sub-command index vector

			std::vector <std::wstring> SubCmds{ ValidSubCommands() };
			std::vector <int> UnorderedSubIndex{};
			std::vector <int> Offsets{};

			for (int i = 0; i < SubCmds.size(); i++) {
				auto Offset{ CurCmd.find(SubCmds.at(i)) };

				if (Offset != std::wstring::npos) {
					UnorderedSubIndex.push_back(i);
					Offsets.push_back(Offset);
				}
			}


			if (UnorderedSubIndex.size() == 0 || Offsets.size() == 0) {

				// No valid sub-commands, but "get" is present - invalid command

				ParsedCommand.CommandIndex = eInvalid;
				return;
			}


			// We now call our lambda which will return a vector of the original offset indices 
			// ordered in correspondence with the offset values sorted from min to max

			auto OrderedOffsets{ OrderVector(Offsets) };


			// Check if the closest sub-command to the "get" keyword is directly after it
			// If it's not, we have invalid junk between "get" and the sub-commands (e.g. disk get afgxsdf serialnumber)

			if (Offsets.at(OrderedOffsets.at(0)) != ValidCommands.at(ParsedCommand.CommandIndex).size() + 3) {
				ParsedCommand.CommandIndex = eInvalid;
				return;
			}


			// Sort our sub-command indices from closest to farthest relative to "get"

			ParsedCommand.SubCommandIndex.resize(UnorderedSubIndex.size());

			for (int i = 0; i < UnorderedSubIndex.size(); i++) {
				ParsedCommand.SubCommandIndex.at(i) = UnorderedSubIndex.at(OrderedOffsets.at(i));
			}
		}
		else if (CurCmd.size() > ValidCommands.at(ParsedCommand.CommandIndex).size()) {

			// There's a valid main command present, but there are no sub-commands and the total size of the command is larger than 
			// the size of the valid main command it contains
			// We have an invalid command (e.g. diskgkdfmkm)

			ParsedCommand.CommandIndex = eInvalid;
		}
	}


	void RespondCommand() {
		ParseCommand();

		// handle sub-commands if present
		if (ParsedCommand.SubCommandIndex.size()) {

			std::wcout << std::endl;

			switch (ParsedCommand.CommandIndex) {

				// Disk Sub-Command (e.g. "disk get serialnumber")
				case eDisk: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {


						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).SerialNumber, eDisk, j);
								}
							} break;

							case 2: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).Model, eDisk, j);
								}
							} break;

							case 3: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).Interface, eDisk, j);
								}
							} break;

							case 4: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).DriveLetter, eDisk, j);
								}
							} break;

							case 5: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).Size, eDisk, j);
								}
							} break;

							case 6: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).FreeSpace, eDisk, j);
								}
							} break;

							case 7: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).MediaType, eDisk, j);
								}
							} break;

							case 8: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(HWID.Disk.at(j).IsBootDrive, eDisk, j);
								}
							} break;

							case 9: {
								for (int j = 0; j < HWID.Disk.size(); j++) {
									FmtPrint(BusType[HWID.Disk.at(j).BusType], eDisk, j);
								}
							} break;

							case 10: {
								for (int i = 0; i < HWID.Disk.size(); i++) {
									HardwareId::DiskObject& Disk{ HWID.Disk.at(i) };

									if (Disk.Volumes.empty() == false) {
										std::wcout << L"Drive: ";
										ChangeTextColor(12);
										std::wcout << Disk.Name.substr(4, Disk.Name.size() - 4);
										ChangeTextColor(15);
										std::wcout << L"\n---------------------";

										for (int j = 0; j < Disk.Volumes.size(); j++) {
											std::wcout << std::endl;
											std::wcout << L"Volume " << j << std::endl;
											std::wcout << L"Name:\t\t\t" << Disk.Volumes.at(j).Name << std::endl;
											std::wcout << L"Serial Number:\t\t" << std::hex << Disk.Volumes.at(j).SerialNumber << std::dec << std::endl;
											std::wcout << L"Size:\t\t\t" << Disk.Volumes.at(j).Size << L" GB" << std::endl;
											std::wcout << L"Free Space:\t\t" << Disk.Volumes.at(j).FreeSpace << L" GB" << std::endl;
											std::wcout << L"Drive Letter:\t\t" << Disk.Volumes.at(j).DriveLetter << std::endl;
										}
									}
									else {
										continue;
									}

									if (i + 1 < HWID.Disk.size()) { 
										std::wcout << std::endl; 
									}
								}
							} break;
						}
					}
				} break;


				// SMBIOS Sub-Command (e.g. "smbios get manufacturer")
				case eSMBIOS: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {
						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								std::wcout << HWID.SMBIOS.SerialNumber << std::endl;
							} break;

							case 2: {
								std::wcout << HWID.SMBIOS.Manufacturer << std::endl;
							} break;

							case 3: {
								std::wcout << HWID.SMBIOS.Product << std::endl;
							} break;

							case 4: {
								std::wcout << HWID.SMBIOS.Version << std::endl;
							} break;
						}
					}
				} break;


				// GPU Sub-Command (e.g. "gpu get resolution")
				case eGPU: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {
						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								for (int i = 0; i < HWID.GPU.size(); i++) {
									FmtPrint(HWID.GPU.at(i).Name, eGPU, i);
								}
							} break;

							case 2: {
								for (int i = 0; i < HWID.GPU.size(); i++) {
									FmtPrint(HWID.GPU.at(i).DriverVersion, eGPU, i);
								}
							} break;

							case 3: {
								for (int i = 0; i < HWID.GPU.size(); i++) {
									std::wstring Res(std::to_wstring(HWID.GPU.at(i).XResolution));
									Res.append(L"x").append(std::to_wstring(HWID.GPU.at(i).YResolution));

									FmtPrint(Res, eGPU, i);
								}
							} break;

							case 4: {
								for (int i = 0; i < HWID.GPU.size(); i++) {
									FmtPrint(HWID.GPU.at(i).RefreshRate, eGPU, i);
								}
							} break;

							case 5: {
								for (int i = 0; i < HWID.GPU.size(); i++) {
									FmtPrint(HWID.GPU.at(i).Memory, eGPU, i, L" GB");
								}
							} break;
						}
					}
				} break;


				// CPU Sub-Command (e.g. "cpu get processorid")
				case eCPU: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {
						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								std::wcout << HWID.CPU.ProcessorId << std::endl;
							} break;

							case 2: {
								std::wcout << HWID.CPU.Manufacturer << std::endl;
							} break;

							case 3: {
								std::wcout << HWID.CPU.Name << std::endl;
							} break;

							case 4: {
								std::wcout << HWID.CPU.Cores << std::endl;
							} break;

							case 5: {
								std::wcout << HWID.CPU.Threads << std::endl;
							} break;
						}
					}

				} break;


				// Network Sub-Command (e.g. "network get mac")
				case eNetwork: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {
						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								for (int i = 0; i < HWID.NetworkAdapter.size(); i++) {
									FmtPrint(HWID.NetworkAdapter.at(i).Name, eNetwork, i);
								}
							} break;

							case 2: {
								for (int i = 0; i < HWID.NetworkAdapter.size(); i++) {
									FmtPrint(HWID.NetworkAdapter.at(i).MAC, eNetwork, i);
								}
							} break;
						}
					}
				} break;


				// System Sub-Command (e.g. "system get osversion")
				case eSystem: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {
						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								std::wcout << HWID.System.Name << std::endl;
							} break;

							case 2: {
								std::wcout << HWID.System.IsHypervisorPresent << std::endl;
							} break;

							case 3: {
								std::wcout << HWID.System.OSVersion << std::endl;
							} break;

							case 4: {
								std::wcout << HWID.System.OSName << std::endl;
							} break;

							case 5: {
								std::wcout << HWID.System.OSArchitecture << std::endl;
							} break;

							case 6: {
								std::wcout << HWID.System.OSSerialNumber << std::endl;
							} break;
						}
					}

				} break;


				// Physcial Memory Sub-Command (e.g. "physicalmemory get partnumber")
				case ePhysicalMemory: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {
						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								std::wcout << HWID.PhysicalMemory.PartNumber << std::endl;
							} break;
						}
					}

				} break;


				// Registry Sub-Command (e.g. "registry get computerhardwareid")
				case eRegistry: {
					for (int i = 0; i < ParsedCommand.SubCommandIndex.size(); i++) {
						switch (ParsedCommand.SubCommandIndex.at(i)) {
							case 1: {
								std::wcout << HWID.Registry.ComputerHardwareId << std::endl;
							} break;
						}
					}

				} break;
			}

			goto ClearCmd;
		}


		// No sub-commands, handle main command
		switch (ParsedCommand.CommandIndex) {

			case eDisk: {
				std::wcout << std::endl;
				PrintDisks();
			} break;

			case eSMBIOS: {
				std::wcout << std::endl;
				PrintSMBIOS();
			} break;

			case eGPU: {
				std::wcout << std::endl;
				PrintGPUs();
			} break;

			case eCPU: {
				std::wcout << std::endl;
				PrintCPU();
			} break;

			case eNetwork: {
				std::wcout << std::endl;
				PrintNetwork();
			} break;

			case eSystem: {
				std::wcout << std::endl;
				PrintSystem();
			} break;

			case ePhysicalMemory: {
				std::wcout << std::endl;
				PrintPhysicalMemory();
			} break;

			case eRegistry: {
				std::wcout << std::endl;
				PrintRegistry();
			} break;

			case eAll: {

				ChangeTextColor();
				std::wcout << L"\n-------------\n";
				std::wcout << L"[+] Disks [+] \n";
				std::wcout << L"-------------\n\n";
				ChangeTextColor(15);
				PrintDisks();

				ChangeTextColor();
				std::wcout << L"\n--------------\n";
				std::wcout << L"[+] SMBIOS [+]\n";
				std::wcout << L"--------------\n\n";
				ChangeTextColor(15);
				PrintSMBIOS();

				ChangeTextColor();
				std::wcout << L"\n------------\n";
				std::wcout << L"[+] GPUs [+]\n";
				std::wcout << L"------------\n\n";
				ChangeTextColor(15);
				PrintGPUs();

				ChangeTextColor();
				std::wcout << L"\n-----------\n";
				std::wcout << L"[+] CPU [+]\n";
				std::wcout << L"-----------\n\n";
				ChangeTextColor(15);
				PrintCPU();

				ChangeTextColor();
				std::wcout << L"\n---------------\n";
				std::wcout << L"[+] Network [+] \n";
				std::wcout << L"---------------\n\n";
				ChangeTextColor(15);
				PrintNetwork();

				ChangeTextColor();;
				std::wcout << L"\n--------------\n";
				std::wcout << L"[+] System [+]\n";
				std::wcout << L"--------------\n\n";
				ChangeTextColor(15);
				PrintSystem();

				ChangeTextColor();
				std::wcout << L"\n-----------------------\n";
				std::wcout << L"[+] Physical Memory [+]\n";
				std::wcout << L"-----------------------\n\n";
				ChangeTextColor(15);
				PrintPhysicalMemory();

				ChangeTextColor();
				std::wcout << L"\n----------------\n";
				std::wcout << L"[+] Registry [+]\n";
				std::wcout << L"----------------\n\n";
				ChangeTextColor(15);
				PrintRegistry();

			} break;

			case eHelp: {
				ShellExecuteW(nullptr, nullptr, L"https://github.com/sondernextdoor/WindowsHardwareInfo#readme", nullptr, nullptr, SW_SHOW);
			} break;

			case eExit: {
				exit(0);
			} break;

			default: {
				std::wcout << L"Invalid Command\n";
			} break;
		}


	ClearCmd:

		ParsedCommand.CommandIndex = eInvalid;
		ParsedCommand.SubCommandIndex.resize(0);
	}


	void Create() {
		auto NewLine{ [] () -> void {
			std::wcout << CmdName;
			std::getline(std::wcin, CurCmd);

			HardwareId::RemoveWhitespaces(CurCmd);
			if (CurCmd.empty()) { return; }

			bool bFmt{
				(CurCmd.find(L"diskget") != std::wstring::npos && HWID.Disk.size() > 1
				|| CurCmd.find(L"gpuget") != std::wstring::npos && HWID.GPU.size() > 1
				|| CurCmd.find(L"networkget") != std::wstring::npos && HWID.NetworkAdapter.size() > 1)
			};

			RespondCommand();
			if (!bFmt) { std::wcout << std::endl; }
		} };

		HWID = HardwareId();

		for (;;) {
			NewLine();
		}
	}
};
