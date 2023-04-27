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
  local evt, evtData = isbd.waitEvent( 1000 )

  if evt ~= nil then
    
    if evt == 0x01 then
      print( string.format( "MT message received, sn=%d, size=%d", 
        evtData.sn, #evtData.payload ) )
    elseif evt == 0x02 then
      print( string.format( "MO message sent, sn=%d", evtData.sn ) )
    elseif evt == 0x03 then
      print( string.format( "Ring alert" ) )
      isbd.initSession({ alert = false })
    elseif evt == 0x04 then
      print( string.format( "Service avaivality: %d", evtData.svca ) )
    elseif evt == 0x05 then
      print( string.format( "Signal quality: %d", evtData.sigq ) )
    elseif evt == 0x0F then
      print( string.format( "Error (%03d)", evtData.err ) )
    else
      print( string.format( "Event (0x%02X) received", evt ) )
    end

  end
end


