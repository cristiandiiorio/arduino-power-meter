import numpy as np
from sklearn.linear_model import LinearRegression

# Sensor and real-life values
sensor_values = np.array([0.231, 0.255, 0.252, 0.269, 0.10, 0.007]).reshape(-1, 1)
real_values = np.array([0.11, 0.12, 0.12, 0.13, 0.00, 0.00])

#Create and fit the linear regression model
model = LinearRegression()
model.fit(sensor_values, real_values)

#Get the parameters of the linear model
a = model.coef_[0]
b = model.intercept_

print(a,b)