function [RMSE, MAE] = calcular_errores(T_real, T_pred)
% CALCULAR_ERRORES Calcula RMSE y MAE entre datos medidos y predichos.
%
%   [RMSE, MAE] = CALCULAR_ERRORES(T_real, T_pred)

    error_real = T_real - T_pred;
    RMSE = sqrt(mean(error_real.^2));
    MAE = mean(abs(error_real));
end
