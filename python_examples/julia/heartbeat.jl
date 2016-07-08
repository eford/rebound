if !isdefined(:heartbeat_jl)

# Find relevant libraries on system
if !isdefined(:LIB_REBOUND)  global const LIB_REBOUND = Libdl.find_library(["librebound.so"],["."]) end
if !isdefined(:LIB_REBOUND_USER)  global const LIB_REBOUND_USER = Libdl.find_library(["librebounduser.so"],["."]) end

# Julia code to make heartbeat_jl
function heartbeat_jl(reb::Ptr{Void})
   dt_update_screen = 20*pi
   if ccall((:reb_output_check,LIB_REBOUND),Cint,(Ptr{Void},Cdouble,),reb,dt_update_screen) >0
     ccall((:reb_output_timing,LIB_REBOUND),Cint,(Ptr{Void},Cdouble,),reb,tend)
   end	  
   dt_update_file = 40*pi
   orbits_filename = "orbits.txt"
   if ccall((:reb_output_check,LIB_REBOUND),Cint,(Ptr{Void},Cdouble,),reb,dt_update_file) >0
     ccall((:reb_integrator_synchronize,LIB_REBOUND),Cint,(Ptr{Void},),reb)
     ccall((:reb_output_orbits,LIB_REBOUND),Cint,(Ptr{Void},Cstring,),reb,orbits_filename)
     ccall((:reb_move_to_com,LIB_REBOUND),Cint,(Ptr{Void},),reb)     
   end   
   convert(Cint,0)
end #  heartbeat_jl

# Get heartbeat_c, a pointer to a C-callable version of the function, heartbeat_jl
if !isdefined(:heartbeat_c)
   global const heartbeat_c = cfunction(heartbeat_jl,Cint,(Ptr{Void},) )
end

function activate_heartbeat(ptr_sim::Ptr{Void})
  # Set simulation to use heartbeat_c
  ccall((:set_heartbeat,LIB_REBOUND_USER),Void,(Ptr{Void},Ptr{Void},),ptr_sim,heartbeat_c)   
end

end # !isdefined(:heartbeat_jl)
