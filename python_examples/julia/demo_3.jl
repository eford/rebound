#const LIB_REBOUND_USER = Libdl.find_library(["librebounduser.so"],["."])
#ccall((:print_args,LIB_REBOUND_USER),Void,(Cint,Cdouble,),1,0.001)

using PyCall
@pyimport rebound
#@pyimport ctypes

sim = rebound.Simulation()
sim[:add](m=1.)
sim[:add](m=2.)

p = convert(Ptr{Void},ctypes.addressof(sim))
ccall((:activate_migration_forces,LIB_REBOUND_USER),Void,(Ptr{Void},),p)

ccall((:set_tau_a,LIB_REBOUND_USER),Void,(Cint,Cdouble,),2,0.001)

ccall((:get_tau_a,LIB_REBOUND_USER),Cdouble,(Cint,),1)


#=

pysim = convert(PyObject,sim)
 
function migration_forces_julia(sim::PyObject)
  ccall((:migration_forces,LIB_REBOUND_USER),Void,(Ptr{Void},),sim)
end
 
migration_forces_c_ptr = cfunction(migration_forces_julia,Void,(Ptr{UInt8},))
=#

