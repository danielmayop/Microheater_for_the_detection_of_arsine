function T_amb = calcular_temp_ambiente(t, T, t_on)
% CALCULAR_TEMP_AMBIENTE Estima la temperatura ambiente previa al encendido.
%
%   T_amb = CALCULAR_TEMP_AMBIENTE(t, T, t_on) promedia todas las
%   muestras de temperatura anteriores al instante de encendido t_on.

    idx_amb = t < t_on;
    T_amb = mean(T(idx_amb));
end
