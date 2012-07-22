#include "platform.h"

#include <cassert>

#include "shared.h"

extern bool Verbose;

String const Platform::Families::Windows = "windows";
String const Platform::Families::Linux = "linux";
String const Platform::Families::BSD = "bsd";
String const Platform::Members::Unknown = "unknown";
String const Platform::Members::Windows2000 = "win2000";
String const Platform::Members::WindowsXP = "winxp";
String const Platform::Members::WindowsServer2003 = "winserver2003";
String const Platform::Members::WindowsVista = "winvista";
String const Platform::Members::WindowsServer2008 = "winserver2008";
String const Platform::Members::Windows7 = "win7";
String const Platform::Members::Windows8 = "win8";
String const Platform::Members::WindowsServer2012 = "winserver2012";
String const Platform::Members::LinuxRedHat = "redhat";
String const Platform::Members::LinuxDebian = "debian";
String const Platform::Members::LinuxUbuntu = "ubuntu";
String const Platform::Members::LinuxArch = "arch";
String const Platform::Members::OpenBSD = "openbsd";
String const Platform::Members::FreeBSD = "freebsd";
String const Platform::Members::NetBSD = "netbsd";
String const Platform::LinuxClasses::Invalid = "invalid";
String const Platform::LinuxClasses::RedHat = "redhat";
String const Platform::LinuxClasses::Debian = "debian";
String const Platform::LinuxClasses::Slackware = "slackware";
String const Platform::LinuxClasses::Arch;

Platform::Platform(void) : Family(Families::Linux), Member(Members::Unknown), ArchitectureBits(sizeof(void *) * 8), LinuxClass(LinuxClasses::Invalid)
{
	std::pair<bool, String> OverrideArchitecture = FindProgramArgument("arch");
	if (OverrideArchitecture.first)
	{
		StringStream Value(OverrideArchitecture.second);
		Value >> ArchitectureBits;
	}

	std::pair<bool, String> OverrideFamily = FindProgramArgument("platform-family"),
		OverrideMember = FindProgramArgument("platform-member");

#ifdef _WIN32
	Family = Families::Windows;
	if (!OverrideMember.first)
	{
		OSVERSIONINFOEX VersionInfo;
		memset(&VersionInfo, 0, sizeof(VersionInfo));
		VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
		GetVersionEx(reinterpret_cast<OSVERSIONINFO *>(&VersionInfo));
		if (VersionInfo.dwMajorVersion == 5)
		{
			if (VersionInfo.dwMinorVersion == 0)
				Member = Members::Windows2000;
			else if ((VersionInfo.dwMinorVersion == 1) || ((VersionInfo.dwMinorVersion == 2) && (VersionInfo.wProductType == VER_NT_WORKSTATION)))
				Member = Members::WindowsXP;
			else Member = Members::WindowsServer2003;
		}
		else if (VersionInfo.dwMajorVersion == 6)
		{
			if (VersionInfo.wProductType == VER_NT_WORKSTATION)
			{
				if (VersionInfo.dwMinorVersion == 0)
					Member = Members::WindowsVista;
				else if (VersionInfo.dwMinorVersion == 1)
					Member = Members::Windows7;
				else if (VersionInfo.dwMinorVersion == 2)
					Member = Members::Windows8;
			}
			else
			{
				if ((VersionInfo.dwMinorVersion == 0) || (VersionInfo.dwMinorVersion == 1))
					Member = Members::WindowsServer2008;
				else if (VersionInfo.dwMinorVersion == 2)
					Member = Members::WindowsServer2012;
			}
		}
	}
#elif __OpenBSD__
	Family = Families::BSD;
	Member = Members::OpenBSD;
#elif __FreeBSD__
	Family = Families::BSD;
	Member = Members::FreeBSD;
#elif __NetBSD__
	Family = Families::BSD;
	Member = Members::NetBSD;
#else
	// Linux
	Family = Families::Linux;
	FileInput LSBRelease(AsNativeString(FilePath("/etc/lsb-release")));
	String LSBReleaseLine;
	while (std::getline(LSBRelease, LSBReleaseLine))
	{
		if (LSBReleaseLine.find("Ubuntu")) Member = Members::LinuxUbuntu;
		else if (LSBReleaseLine.find("Debian")) Member = Members::LinuxDebian;
	}

	if (!Member.empty()) {}
	else if (FilePath("/etc/arch-release").Exists())
	{
		Member = Members::LinuxArch;
	}
#endif
	if (OverrideFamily.first) Family = OverrideFamily.second;
	if (OverrideMember.first) Member = OverrideMember.second;
	if (Family == Families::Linux)
	{
		if (Member == Members::LinuxUbuntu) LinuxClass = LinuxClasses::Debian;
		else if (Member == Members::LinuxDebian) LinuxClass = LinuxClasses::Debian;
		else if (Member == Members::LinuxArch) LinuxClass = LinuxClasses::Arch;
	}
	assert(!Family.empty());
	if (Verbose)
		std::cout << "Determined platform: " << Family << ", " << Member << ", " << ArchitectureBits << "-bit" << std::endl;
}

String Platform::GetIdentifier(void) { return "platform"; }

static void PrintEnumerations(std::ostream &Out)
{
	Out << "\tAllowed FAMILY and MEMBER values follow in the format \"family: member1, member2...\".\n"
		"\t" << Platform::Families::Windows << ": " << Platform::Members::Windows2000 << ", " << Platform::Members::WindowsXP << ", " << Platform::Members::WindowsServer2003 << ", " << Platform::Members::WindowsVista << ", " << Platform::Members::WindowsServer2008 << ", " << Platform::Members::Windows7 << ", " << Platform::Members::Windows8 << ", " << Platform::Members::WindowsServer2012 << "\n"
		"\t" << Platform::Families::Linux << ": " << Platform::Members::LinuxDebian << ", " << Platform::Members::LinuxArch << "\n"
		"\t" << Platform::Families::BSD << ": " << Platform::Members::OpenBSD << ", " << Platform::Members::FreeBSD << ", " << Platform::Members::NetBSD << "\n";

}
		
void Platform::DisplayControllerHelp(std::ostream &Out)
{
	Out << "\t" << GetIdentifier() << "\n"
		"\tResult: FAMILY MEMBER ARCH\n"
		"\tDetermines the target operating system and architecture.  FAMILY specifies broadly the type of operating system, whereas MEMBER is the specific distribution or generation of the family.  ARCH is the maximum bit depth for memory addresses on the system, such as 32 or 64.\n";
	PrintEnumerations(Out);
	Out << "\tarch=32|64\n"
		"Overrides the detected architecture.  This may also affect installation directories and the like.\n"
		"\n";
}

void Platform::DisplayUserHelp(std::queue<String> &&Arguments, std::ostream &Out)
{
	Out << "\tplatform-family=FAMILY\n"
		"\tplatform-member=MEMBER\n"
		"\tOverrides the detected platform family and member with FAMILY and MEMBER.\n";
	PrintEnumerations(Out);
	Out << "\n";
	Out << "\tarch=32|64\n"
		"\tOverrides the detected architecture.  This may also affect installation directories and the like.\n"
		"\n";
}

void Platform::Respond(std::queue<String> &&Arguments, std::ostream &Out)
{
	Out << Family << " " << Member << " " << ArchitectureBits << "\n";
}

String const &Platform::GetFamily(void) const { return Family; }

String const &Platform::GetMember(void) const { return Member; }
		
unsigned int Platform::GetArchitectureBits(void) const { return ArchitectureBits; }

String const &Platform::GetLinuxClass(void) const 
{ 
	assert(Family == Families::Linux);
	return LinuxClass; 
}

