print( "Example of Iridium SBD application using Lua" );

local result = isbd.setup()

if result == 0 then
  print( "Iridium SBD service setup OK" );

  -- Sends a message with a given maximum of retries
  isbd.sendMessage( "This message was sent from Lua!", 10 );

else
  print( "Iridium SBD service setup failed" );
end

while true do
  -- Waits until a event is captured or timeout is triggered
  -- This function can be non blocking if called with a timeout of 0
  local evt, data = isbd.waitEvent( 1000 )

  if evt ~= nil then
    print( string.format( "Event (%03d) received", evt ) )
  end
end


