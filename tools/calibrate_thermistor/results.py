
import numpy as np
import scipy as sp

from matplotlib import pyplot as plt

from scipy.optimize import curve_fit


# document to determine Steinhart–Hart parameters for thermistor

results = np.loadtxt("experimental_data.txt")


resistances = results[:,0]
temps = results[:,1]

def steinhart_hart(x, a, b, c):
    return 1/(a + b*np.log(x) + c*np.log(x)**3)

# initial guess are common values for 100k thermistor
p0 = [
    1.009249522e-03,
    2.378405444e-04,
    2.019202697e-07,
]

coefficients, covariance = curve_fit(steinhart_hart, resistances, temps, p0, bounds = (0, np.inf))


plt.plot(resistances, temps, 'o')

x = np.linspace(50, 100000, 1000)
plt.plot(x, steinhart_hart(x, *coefficients))
print("Steinhart-Hart coefficients: ", coefficients)

MSE = np.sum((temps - steinhart_hart(resistances, *coefficients))**2)/len(temps)
print("MSE: {}".format(MSE))

## compare to simple log fit

def log_fit(x, a, b):
    return a + b*np.log(x)

coefficients, covariance = curve_fit(log_fit, resistances, temps)

print("Log fit coefficients: ", coefficients)
MSE = np.sum((temps - log_fit(resistances, *coefficients))**2)/len(temps)
print("MSE: {}".format(MSE))
plt.plot(x, log_fit(x, *coefficients))

# set x scale to log
plt.xscale('log')
plt.grid(True, which="both", ls="-", color='0.65')

plt.xlabel("Resistance (Ohms)")
plt.ylabel("Temperature (C)")

plt.show()