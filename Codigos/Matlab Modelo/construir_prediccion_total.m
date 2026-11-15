function T_pred_total = construir_prediccion_total(t, T_amb, t_on, t_off, K_on, tau_on, T0_cool, tau_off)
% CONSTRUIR_PREDICCION_TOTAL Arma la predicción del modelo a lo largo de
% todo el experimento (antes del encendido, calentamiento, enfriamiento).
%
%   Entradas:
%       t       - vector de tiempo completo
%       T_amb   - temperatura ambiente
%       t_on    - instante de encendido [s]
%       t_off   - instante de apagado [s]
%       K_on, tau_on   - parámetros del modelo de calentamiento
%       T0_cool, tau_off - parámetros del modelo de enfriamiento
%
%   Salida:
%       T_pred_total - predicción del modelo para todo el vector t

    T_pred_total = zeros(size(t));

    % Antes del encendido
    idx1 = t < t_on;
    T_pred_total(idx1) = T_amb;

    % Durante el calentamiento
    idx2 = (t >= t_on) & (t <= t_off);
    tiempo_rel_heat = t(idx2) - t_on;
    T_pred_total(idx2) = T_amb + K_on*(1 - exp(-tiempo_rel_heat/tau_on));

    % Durante el enfriamiento
    idx3 = t > t_off;
    tiempo_rel_cool = t(idx3) - t_off;
    T_pred_total(idx3) = T_amb + (T0_cool - T_amb)*exp(-tiempo_rel_cool/tau_off);
end
