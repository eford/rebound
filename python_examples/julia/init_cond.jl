const mass_mars = 6.39e23
const mass_earth = 5.972e24
const mass_sun = 1.989e30

function draw_embryo_mu()
 mu_mars = mass_mars/mass_sun
 mu_min = 0.5*mu_mars
 mu_max = 2.0*mu_mars
 mu_min+(mu_max-mu_min)*rand()
end

function generate_embryo_mu_list(mtot::Real)
  msum = 0.0
  mulist = zeros(0)
  while msum < mtot
    mu = draw_embryo_mu()
    push!(mulist,mu)
    msum += mu
  end
  mulist .*= mtot/msum
  return mulist
end

draw_from_power_law(alpha::Real, ll::Real, ul::Real) =  ((ul^(alpha+1)-ll^(alpha+1))*rand() + ll^(alpha+1))^(1/(alpha+1))

function generate_embryo_draw_semimajor_axis_list(alpha_surface_density::Real, ll::Real, ul::Real; n::Integer = 1)
  alpha_mass_enclosed = alpha_surface_density + 2
  alist = [draw_from_power_law(alpha_mass_enclosed ,ll,ul) for i in 1:n]
  sort!(alist)
end

using Base.Test
#function run_tests()
  target_mass = 10.* mass_earth/mass_sun
  mulist = generate_embryo_mu_list(target_mass)
  @test_approx_eq sum(mulist) target_mass

  alpha = -1.5
  min_a = 1.0
  max_a = 10.0
  alist = generate_embryo_draw_semimajor_axis_list(alpha,min_a,max_a,n=length(mulist))
  @test issorted(alist)
  @test min_a <= minimum(alist) <= maximum(alist) <= max_a
#end

#run_tests()

