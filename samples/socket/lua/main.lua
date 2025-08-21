host = "*"
port = 9000

udp = assert(socket.udp())

assert(udp:setsockname(host, port))
assert(udp:settimeout(10))

ip, port = udp:getsockname()
assert(ip, port)

print("Waiting packets on " .. ip .. ":" .. port)
while 1 do
	dgram, ip, port = udp:receivefrom()
	if dgram then
		print("Echoing back to " .. ip .. ":" .. port)
		udp:sendto(dgram, ip, port)
	else
        print(ip)
    end
end