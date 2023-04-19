print( "Example of Iridium SBD application using Lua" );

local result = iridium.isbd_setup()

if result == 0 then
  print( "Iridium SBD serviec OK" );
else
  print( "Iridium SBD service failed" );
end

while true do
  local evt, data = iridium.isbd_wait_event( 1000 )
  if evt ~= nil then
    print( string.format( "Event (%03d) received -> %s", evt, tostring( data ) ) )
  end
end


