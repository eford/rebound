#const LIB_REBOUND_USER = Libdl.find_library(["librebounduser.so"],["."])
#ccall((:print_args,LIB_REBOUND_USER),Void,(Cint,Cdouble,),1,0.001)

using PyCall
@pyimport rebound
@pyimport ctypes

sim = rebound.Simulation()
sim[:add](m=1.)
sim[:add](m=0.001,a=1.0,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.3,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.6,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())
sim[:add](m=0.001,a=1.9,e=0.001,inc=0.001,omega=2pi*rand(),Omega=2pi*rand(),M=2pi*rand())

sim[:move_to_com]()
tend = 10*2pi

@time sim[:integrate](tend)


