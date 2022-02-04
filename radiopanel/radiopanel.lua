-- NAV1/2
-- NAV<1,2>_RADIO_FRACT_INC/DEC_CARRY
-- NAV<1,2>_RADIO_WHOLE_INC/DEC
-- NAV<1,2>_RADIO_SWAP

-- COM1/2
-- COM<1,2>_RADIO_FRACT_INC/DEC_CARRY
-- COM<1,2>_RADIO_WHOLE_INC/DEC
-- COM<1,2>_RADIO_SWAP

-- AP_MANAGED_SPEED_IN_MACH_TOGGLE

-- XPDR
-- XPNDR_1000_INC/DEC
-- XPNDR_100_INC/DEC
-- XPNDR_10_INC/DEC
-- XPNDR_1_INC/DEC
-- XPNDR_IDENT_TOGGLE or XPNDR_IDENT_ON


-- AP1
-- AP_SPD_VAR_INC/DEC
-- AP_MACH_VAR_INC/DEC

-- AP_VS_VAR_INC/DEC && AP_PITCH_REF_INC_DN/UP
-- AP_ALT_VAR_INC/DEC

-- AP_VS_ON/OFF/SET???
-- AP_FLIGHT_LEVEL_CHANGE (_ON/OFF???)


-- AP2
-- AP_MAX_BANK_INC/DEC
-- HEADING_BUG_INC/DEC
-- VOR<1,2>_OBI_INC/DEC
-- VOR<1,2>_OBI_FAST_INC/DEC
-- AP_PANEL_HEADING_HOLD
-- AP_NAV1_HOLD


-- AUX1
-- KOHLSMAN_INC/DEC
-- BAROMETRIC_STD_PRESSURE


-- G1000_MFD_ZOOMIN/OUT_BUTTON
-- G1000_MFD_GROUP_KNOB_INC/DEC
-- G1000_MFD_PAGE_KNOB_INC/DEC
-- G1000_MFD_ENTER_BUTTON
-- G1000_MFD_CURSOR_BUTTON

-- returns a lambda the will read and write back an updated value in MSFS
-- Multiplier will multiply amount before adding to original
function fsuipc_send_adjust_event(offset,input_modifier,modifier)
   return(function(amount)
	 local new_amount = amount
	 if (new_amount == nil) then
	    new_amount = 1
	 end

	 if (input_modifier ~= nil) then  new_amount = input_modifer(new_amount) end
	 local next_value = 0
	 if (modifier ~= nil) then next_value = modifier(next_value) end
	    
	 print(offset .. " Getting current setting " )
	 print(offset .. " Adding " .. new_amount) 
   end)
end


-- returns a lambda the will read and write back an updated value in MSFS
function simclient_send_incremental_event(event_dec,event_inc)
   return(function(amount)
	 local new_amount = amount
	 if (new_amount == nil) then new_amount = -1 end
	 
	 if (new_amount < 0) then
	    print(event_dec)
	    -- lpc_fs_event(event_dec)
	 end
	 if (new_amount > 0) then
	    print(event_inc)
	    -- lpc_fs_event(event_inc)
	 end
   end)
end

function simclient_send_event(event)
   return(function()
	 print(event .. " Sent simclient event")
   end
   )
end


function multiply(factor)
   return(
      function(arg)
	 return(arg * factor)
      end
   )
end   

function wrap(amount)
   return(
      function(arg)
	 return(arg % amount)
      end
   )
end   


      
msfs_event_mapping = { 
   {
      button1=simclient_send_event("COM1_RADIO_SWAP"),
      button2=simclient_send_event("COM2_RADIO_SWAP"),
      encoders={
	 simclient_send_incremental_event("COM1_RADIO_WHOLE_DEC","COM1_RADIO_WHOLE_INC"), -- LARGE DIAL
	 simclient_send_incremental_event("COM1_RADIO_FRACT_DEC_CARRY","COM1_RADIO_FRACT_INC_CARRY"),
	 simclient_send_incremental_event("COM2_RADIO_WHOLE_DEC","COM2_RADIO_WHOLE_INC"), -- LARGE DIAL
	 simclient_send_incremental_event("COM2_RADIO_FRACT_DEC_CARRY","COM2_RADIO_FRACT_INC_CARRY")
      }
   },
   {
      button1=simclient_send_event("NAV1_RADIO_SWAP"),
      button2=simclient_send_event("NAV2_RADIO_SWAP"),
      encoders={
	 simclient_send_incremental_event("NAV1_RADIO_WHOLE_DEC","NAV1_RADIO_WHOLE_INC"), -- LARGE DIAL
	 simclient_send_incremental_event("NAV1_RADIO_FRACT_DEC_CARRY","NAV1_RADIO_FRACT_INC_CARRY"),
	 simclient_send_incremental_event("NAV2_RADIO_WHOLE_DEC","NAV2_RADIO_WHOLE_INC"), -- LARGE DIAL
	 simclient_send_incremental_event("NAV2_RADIO_FRACT_DEC_CARRY","NAV2_RADIO_FRACT_INC_CARRY"),
      }
   },
   {
      button1=simclient_send_event("AP_FLIGHT_LEVEL_CHANGE"),
      button2=simclient_send_event("AP_VS_ON"), -- is this correct?
      encoders={
	 simclient_send_incremental_event("AP_MANAGED_SPEED_IN_MACH_OFF","AP_MANAGED_SPEED_IN_MACH_ON")
	 fsuipc_send_adjust_event(0x07E2,multiply(1)),  -- 07E2 2 Autopilot airspeed value, in knots
	 fsuipc_send_adjust_event(0x07F2,multiply(25)), -- 07F2 2 Autopilot vertical speed value, as ft/min
	 fsuipc_send_adjust_event(0x07D4,multiply(65536*.3048))    -- 07D4 4 Autopilot altitude value, as metres*65536
      }
   },
   {
      button1=simclient_send_event("AP_PANEL_HEADING_HOLD"),
      button2=simclient_send_event("AP_NAV1_HOLD"),
      encoders={
	 sim_client_send_incremental_event("AP_MAX_BANK_DEC","AP_MAX_BANK_INC"),
	 fsuipc_send_adjust_event(0x07CC,nil,wrap(360)),   -- 07CC 2 Autopilot heading value, as degrees*65536/360
	 fsuipc_send_adjust_event(0x0C5E,nil,wrap(360)),   -- 0C5E 2 NAV2 OBS setting (degrees, 0359)
	 fsuipc_send_adjust_event(0x0C4E,nil,wrap(360))    -- 0C4E 2 NAV1 OBS setting (degrees, 0359)
      }
   },
   {
      button1=simclient_send_event("XPNDR_IDENT_ON"),
      button2=fsuipc_send_adjust_event(0x0B46,nil,wrap(4))
      encoders={
	 simclient_send_incremental_event("XPNDR_1000_DEC","XPNDR_1000_INC"),
	 simclient_send_incremental_event("XPNDR_100_DEC","XPNDR_100_INC"),
	 simclient_send_incremental_event("XPNDR_10_DEC","XPNDR_10_INC"),
	 simclient_send_incremental_event("XPNDR_1_DEC","XPNDR_1_INC")
      }
   },
   {
      button1=simclient_send_event("XPNDR_IDENT_ON"),
      button2=fsuipc_send_incremental_event(0x0B46,nil,wrap(4))
      encoders={
	 simclient_send_incremental_event("XPNDR_1000_DEC","XPNDR_1000_INC"),
	 simclient_send_incremental_event("XPNDR_100_DEC","XPNDR_100_INC"),
	 simclient_send_incremental_event("XPNDR_10_DEC","XPNDR_10_INC"),
	 simclient_send_incremental_event("XPNDR_1_DEC","XPNDR_1_INC")
      }
   }
}   
	    
	

      
current_mode = 0;

lmc_device_set_name("KnobPanel","86A87FC0:77AD:11EC:8002444553540000");
lmc_print_devices();


lmc_set_handler('KnobPanel',
  function(button, direction, ts)
     local mode_definition = msfs_event_mapping[current_mode]
     if (direction == 1) then
       if (button == 0) then
	  mode_defintion["button1"]()
       end
       if (button == 11) then
	  mode_defintion["button2"]()
       end

       if (button == 22) then current_mode = 1; end -- COM
       if (button == 23) then current_mode = 2; end -- NAV
       if (button == 24) then current_mode = 3; end -- AP1
       if (button == 25) then current_mode = 4; end -- AP2
       if (button == 26) then current_mode = 5; end -- XPDR
       if (button == 27) then current_mode = 6; end -- AUX

    else
       -- button release
       if (button == 1 or
           button == 2 or
           button == 6 or
           button == 7 or
           button == 12 or
           button == 13 or
           button == 17 or
           button == 18) then


	  local encoder_value = {0,0,0,0}
	  if (button == 1 or button == 2) then
	     encoder_value[1] = get_value(4,button == 2)
	  end
	  if (button == 6 or button == 7) then
             encoder_value[2] = get_value(9,button == 7)
	  end
	  if (button == 12 or button == 13) then
	     encoder_value[3] = get_value(15,button == 13)
	  end
	  if (button == 17 or button == 18) then
	     encoder_value[4] = get_value(20,button == 18)
	  end
	  
	  for knob=1,4 do
	     if (encoder_value[knob] ~= 0) do
		mode_definition["encoders"][knob](encoder_value[knob])
	     end
	     end
	  
       end

    end
  end
)

function get_value(offset,direction)
  local value = 1

  if (lmc_get_button('KnobPanel',offset+2) > 0) then value = value + 1 end
  if (lmc_get_button('KnobPanel',offset+1) > 0) then value = value + 2 end
  if (lmc_get_button('KnobPanel',offset) > 0) then value = value + 4 end
  if (not direction) then value = value * -1     end
  return(value)
end




