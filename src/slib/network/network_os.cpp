#include "../../../inc/slib/network/os.h"
#include "../../../inc/slib/network/socket.h"

#include "../../../inc/slib/core/map.h"

SLIB_NETWORK_NAMESPACE_BEGIN

List<IPv4Address> Network::findAllIPv4Addresses()
{
	List<IPv4Address> list;
	ListLocker<NetworkInterfaceInfo> devices(Network::findAllInterfaces());
	for (sl_size i = 0; i < devices.count(); i++) {
		ListLocker<IPv4AddressInfo> addrs(devices[i].addresses_IPv4);
		for (sl_size k = 0; k < addrs.count(); k++) {
			if (addrs[k].address.isHost()) {
				list.add(addrs[k].address);
			}
		}
	}
	return list;
}

List<IPv4AddressInfo> Network::findAllIPv4AddressInfos()
{
	List<IPv4AddressInfo> list;
	ListLocker<NetworkInterfaceInfo> devices(Network::findAllInterfaces());
	for (sl_size i = 0; i < devices.count(); i++) {
		ListLocker<IPv4AddressInfo> addrs(devices[i].addresses_IPv4);
		for (sl_size k = 0; k < addrs.count(); k++) {
			if (addrs[k].address.isHost()) {
				list.add(addrs[k]);
			}
		}
	}
	return list;
}

List<MacAddress> Network::findAllMacAddresses()
{
	List<MacAddress> list;
	ListLocker<NetworkInterfaceInfo> devices(Network::findAllInterfaces());
	for (sl_size i = 0; i < devices.count(); i++) {
		if (devices[i].macAddress.isNotZero()) {
			list.add(devices[i].macAddress);
		}
	}
	return list;
}


sl_bool Network::findInterface(const String& _name, NetworkInterfaceInfo* pInfo)
{
	String name = _name;
	ListLocker<NetworkInterfaceInfo> devices(Network::findAllInterfaces());
	for (sl_size i = 0; i < devices.count(); i++) {
		if (devices[i].displayName == name) {
			if(pInfo) {
				*pInfo = devices[i];				
			}
			return sl_true;
		}
	}
	return sl_false;
}
SLIB_NETWORK_NAMESPACE_END

#if defined(SLIB_PLATFORM_IS_WINDOWS)
#if defined(SLIB_PLATFORM_IS_DESKTOP)
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <netioapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

SLIB_NETWORK_NAMESPACE_BEGIN
template <>
int Compare<MIB_IPADDRROW, IPv4Address>::compare(const MIB_IPADDRROW& a, const IPv4Address& b)
{
	return Compare<sl_uint32>::compare(a.dwAddr, Endian::swap32LE(b.toInt()));
}
template <>
sl_bool Compare<MIB_IPADDRROW, IPv4Address>::equals(const MIB_IPADDRROW& a, const IPv4Address& b)
{
	return Compare<sl_uint32>::equals(a.dwAddr, Endian::swap32LE(b.toInt()));
}

List<NetworkInterfaceInfo> Network::findAllInterfaces()
{
	Socket::initializeSocket();

	List<NetworkInterfaceInfo> ret;
	ULONG ulOutBufLen;

	MIB_IPADDRTABLE* iptable = 0;
	ulOutBufLen = 0;
	if (GetIpAddrTable(iptable, &ulOutBufLen, TRUE) == ERROR_INSUFFICIENT_BUFFER) {
		iptable = (MIB_IPADDRTABLE*)(Base::createMemory(ulOutBufLen));
	}
	if (iptable) {
		if (GetIpAddrTable(iptable, &ulOutBufLen, TRUE) == NO_ERROR) {
			IP_ADAPTER_ADDRESSES* pinfo = 0;
			ulOutBufLen = 0;
			if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pinfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
			{
				pinfo = (IP_ADAPTER_ADDRESSES*)(Base::createMemory(ulOutBufLen));
			}
			if (pinfo) {
				if (GetAdaptersAddresses(AF_UNSPEC, 0, NULL, pinfo, &ulOutBufLen) == NO_ERROR)
				{
					IP_ADAPTER_ADDRESSES* adapter = pinfo;
					while (adapter) {
						NetworkInterfaceInfo device;
						device.name = adapter->AdapterName;
						device.displayName = adapter->FriendlyName;
						device.description = adapter->Description;
						
						IP_ADAPTER_UNICAST_ADDRESS* pip = adapter->FirstUnicastAddress;
						while (pip) {
							SocketAddress sa;
							sa.setSystemSocketAddress(pip->Address.lpSockaddr, pip->Address.iSockaddrLength);
							if (sa.ip.isIPv4()) {
								IPv4AddressInfo a4;
								a4.address = sa.ip.getIPv4();
								sl_uint32 networkPrefixLength = 0;
								if (pip->Length == sizeof(IP_ADAPTER_UNICAST_ADDRESS_LH)) {
									networkPrefixLength = ((IP_ADAPTER_UNICAST_ADDRESS_LH*)pip)->OnLinkPrefixLength;
								} else {
									sl_size indexTable = 0;
									if (BinarySearch<MIB_IPADDRROW, IPv4Address>::search(iptable->table, iptable->dwNumEntries, a4.address, &indexTable)) {
										networkPrefixLength = IPv4Address(Endian::swap32LE(iptable->table[indexTable].dwMask)).getNetworkPrefixLengthFromMask();
									}
								}
								a4.networkPrefixLength = networkPrefixLength;
								device.addresses_IPv4.add(a4);
							} else if (sa.ip.isIPv6()) {
								IPv6AddressInfo a6;
								a6.address = sa.ip.getIPv6();
								device.addresses_IPv6.add(a6);
							}
							pip = pip->Next;
						}
						if (adapter->PhysicalAddressLength == 6) {
							device.macAddress.set(adapter->PhysicalAddress);
						} else {
							device.macAddress.setZero();
						}
						ret.add(device);
						adapter = adapter->Next;
					}
				}
				Base::freeMemory(pinfo);
			}
		}

		Base::freeMemory(iptable);
	}
	return ret;
}

SLIB_NETWORK_NAMESPACE_END

#endif

#elif defined(SLIB_PLATFORM_IS_ANDROID)

#include "../../../inc/slib/core/platform_android.h"

SLIB_NETWORK_NAMESPACE_BEGIN

SLIB_JNI_BEGIN_CLASS(_NetworkDevice, "slib/platform/android/network/NetworkDevice")
	SLIB_JNI_STRING_FIELD(name);
	SLIB_JNI_STRING_FIELD(macAddress);
	SLIB_JNI_OBJECT_FIELD(addresses_IPv4, "[Ljava/lang/String;");
	SLIB_JNI_OBJECT_FIELD(addresses_IPv6, "[Ljava/lang/String;");
SLIB_JNI_END_CLASS

SLIB_JNI_BEGIN_CLASS(_NetworkAddress, "slib/platform/android/network/Network")
	SLIB_JNI_STATIC_METHOD(getAllDevices, "getAllDevices", "()[Lslib/platform/android/network/NetworkDevice;");
SLIB_JNI_END_CLASS

List<NetworkInterfaceInfo> Network::findAllInterfaces()
{
	List<NetworkInterfaceInfo> ret;
	if (_NetworkAddress::get().isNotNull() && _NetworkDevice::get().isNotNull()) {
		JniLocal<jobjectArray> jarr = (jobjectArray)(_NetworkAddress::getAllDevices.callObject(sl_null));
		if (jarr.isNotNull()) {
			sl_uint32 n = Jni::getArrayLength(jarr);
			for (sl_uint32 i = 0; i < n; i++) {
				JniLocal<jobject> jdev = Jni::getObjectArrayElement(jarr, i);
				if (jdev.isNotNull()) {
					NetworkInterfaceInfo dev;
					dev.name = _NetworkDevice::name.get(jdev);
					dev.displayName = dev.name;
					dev.macAddress.setZero();
					dev.macAddress.parse(_NetworkDevice::macAddress.get(jdev));
					JniLocal<jobjectArray> jarrIPv4 = (jobjectArray)(_NetworkDevice::addresses_IPv4.get(jdev));
					if (jarrIPv4.isNotNull()) {
						sl_uint32 nAddr = Jni::getArrayLength(jarrIPv4);
						for (sl_uint32 k = 0; k < nAddr; k++) {
							String saddr = Jni::getStringArrayElement(jarrIPv4, k);
							sl_int32 indexPrefix = saddr.indexOf("/");
							if (indexPrefix > 0) {
								IPv4AddressInfo ip;
								if (ip.address.parse(saddr.substring(0, indexPrefix))) {
									ip.networkPrefixLength = saddr.substring(indexPrefix+1).parseUint32();
									dev.addresses_IPv4.add(ip);
								}
							}
						}

					}
					JniLocal<jobjectArray> jarrIPv6 = (jobjectArray)(_NetworkDevice::addresses_IPv6.get(jdev));
					if (jarrIPv6.isNotNull()) {
						sl_uint32 nAddr = Jni::getArrayLength(jarrIPv6);
						for (sl_uint32 k = 0; k < nAddr; k++) {
							String saddr = Jni::getStringArrayElement(jarrIPv6, k);
							sl_int32 indexPrefix = saddr.indexOf("/");
							if (indexPrefix > 0) {
								IPv6AddressInfo ip;
								if (ip.address.parse(saddr.substring(0, indexPrefix))) {
									dev.addresses_IPv6.add(ip);
								}
							}
						}
					}
					ret.add(dev);
				}
			}
		}
	}
	return ret;
}
SLIB_NETWORK_NAMESPACE_END

#elif defined(SLIB_PLATFORM_IS_UNIX)

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>

#if defined(SLIB_PLATFORM_IS_APPLE)
#define	IFT_ETHER	0x6		/* Ethernet CSMACD */
#include <net/if_dl.h>
#endif
#if defined(SLIB_PLATFORM_IS_LINUX)
#include <linux/if_packet.h>
#endif

SLIB_NETWORK_NAMESPACE_BEGIN
List<NetworkInterfaceInfo> Network::findAllInterfaces()
{
	Map<String, NetworkInterfaceInfo> ret;

    struct ifaddrs * adapters = 0;
    getifaddrs(&adapters);

	if (adapters) {
		
		struct ifaddrs* adapter = adapters;
		
		while (adapter) {
			
			String name = adapter->ifa_name;
			
			NetworkInterfaceInfo* pdev = ret.getItemPtr(name);
			if (!pdev) {
				NetworkInterfaceInfo dev;
				dev.name = name;
				dev.displayName = name;
				dev.macAddress.setZero();
				ret.put(name, dev);
				pdev = ret.getItemPtr(name);
			}
			if (pdev && adapter->ifa_addr) {
				if (adapter->ifa_addr->sa_family == AF_INET) {
					// ipv4 address
					sockaddr_in* addr = (sockaddr_in*)(adapter->ifa_addr);
					sockaddr_in* mask = (sockaddr_in*)(adapter->ifa_netmask);
					IPv4AddressInfo ip;
					ip.address = IPv4Address(Endian::swap32LE(addr->sin_addr.s_addr));
					ip.networkPrefixLength = IPv4Address(Endian::swap32LE(mask->sin_addr.s_addr)).getNetworkPrefixLengthFromMask();
					pdev->addresses_IPv4.add(ip);
				} else if (adapter->ifa_addr->sa_family == AF_INET6) {
					// ipv6 address
					SocketAddress s;
					s.setSystemSocketAddress(adapter->ifa_addr);
					IPv6AddressInfo ip;
					ip.address = s.ip.getIPv6();
					pdev->addresses_IPv6.add(ip);
				}
#if defined(SLIB_PLATFORM_IS_APPLE)
				else if (adapter->ifa_addr->sa_family == AF_LINK) {
					sockaddr_dl* addr = (sockaddr_dl*)(adapter->ifa_addr);
					if (addr->sdl_type == IFT_ETHER) {
						sl_uint8* base = (sl_uint8*)(addr->sdl_data + addr->sdl_nlen);
						pdev->macAddress.set(base);
					}
				}
#endif
#if defined(SLIB_PLATFORM_IS_LINUX)
				else if (adapter->ifa_addr->sa_family == AF_PACKET) {
					sockaddr_ll* addr = (sockaddr_ll*)(adapter->ifa_addr);
					if (addr->sll_halen == 6) {
						pdev->macAddress.set((sl_uint8*)(addr->sll_addr));
					}
				}
#endif
			}
			adapter = adapter->ifa_next;
		}
		freeifaddrs(adapters);
	}

    return ret.values();
}
SLIB_NETWORK_NAMESPACE_END

#endif

#if defined(SLIB_PLATFORM_IS_UNIX)
#include <netdb.h>
#include <net/if.h>
#endif

SLIB_NETWORK_NAMESPACE_BEGIN
sl_uint32 Network::getInterfaceIndexFromName(const String& _name)
{
#if defined(SLIB_PLATFORM_IS_WINDOWS)
	Socket::initializeSocket();
#endif
	String8 name = _name;
	sl_uint32 n = if_nametoindex(name.getBuf());
	return n;
}

String Network::getInterfaceNameFromIndex(sl_uint32 index)
{
#if defined(SLIB_PLATFORM_IS_WINDOWS)
	Socket::initializeSocket();
#endif
	char buf[256];
	char* s = if_indextoname(index, buf);
	if (s) {
		return String(s);
	} else {
		return String::null();
	}
}

List<IPAddress> Network::getIPAddressesFromHostName(const String& _hostName)
{
	Socket::initializeSocket();

	List<IPAddress> ret;

	String8 hostName = _hostName;
	addrinfo *addrs = sl_null;

	SocketAddress sa;
	int iRet = getaddrinfo(hostName.getBuf(), sl_null, sl_null, &addrs);
	if (iRet == 0) {
		addrinfo* addr = addrs;
		while (addr) {
			sl_int32 lenAddr = (sl_int32)(addr->ai_addrlen);
			if (lenAddr > 0) {
				sa.ip.setNone();
				sa.setSystemSocketAddress(addr->ai_addr, lenAddr);
				if (sa.ip.isNotNone()) {
					ret.add(sa.ip);
				}
			}
			addr = addr->ai_next;
		}
		if (addrs) {
			freeaddrinfo(addrs);
		}
	}
	return ret;
}

IPAddress Network::getIPAddressFromHostName(const String& hostName)
{
	ListLocker<IPAddress> list(getIPAddressesFromHostName(hostName));
	sl_size i;
	for (i = 0; i < list.count(); i++) {
		if (list[i].isIPv4()) {
			return list[i];
		}
	}
	for (i = 0; i < list.count(); i++) {
		if (list[i].isIPv6()) {
			return list[i];
		}
	}
	return IPAddress::none();
}

IPv4Address Network::getIPv4AddressFromHostName(const String& hostName)
{
	ListLocker<IPAddress> list(getIPAddressesFromHostName(hostName));
	for (sl_size i = 0; i < list.count(); i++) {
		if (list[i].isIPv4()) {
			return list[i].getIPv4();
		}
	}
	return IPv4Address::any();
}

IPv6Address Network::getIPv6AddressFromHostName(const String& hostName)
{
	ListLocker<IPAddress> list(getIPAddressesFromHostName(hostName));
	for (sl_size i = 0; i < list.count(); i++) {
		if (list[i].isIPv6()) {
			return list[i].getIPv6();
		}
	}
	return IPv6Address::any();
}

SLIB_NETWORK_NAMESPACE_END
