function [tau_off, T_pred_cool] = ajustar_modelo_cool(t_cool_rel, T_cool, T_amb, T0_cool, tau_off0)
% AJUSTAR_MODELO_COOL Ajusta el modelo de enfriamiento por mínimos
% cuadrados no lineales (fminsearch).
%
%   Modelo: T(t) = T_amb + (T0-T_amb)*exp(-t/tau_off)
%
%   Entradas:
%       t_cool_rel - tiempo relativo del tramo de enfriamiento
%       T_cool     - temperatura medida del tramo de enfriamiento
%       T_amb      - temperatura ambiente
%       T0_cool    - temperatura inicial del enfriamiento
%       tau_off0   - valor inicial de tau_off
%
%   Salidas:
%       tau_off     - constante de tiempo de enfriamiento ajustada
%       T_pred_cool - temperatura predicha por el modelo ajustado
%
%   NOTA: en el script original había un error de paréntesis en esta
%   ecuación:
%       T_pred_cool = T_amb + (T0_cool-T_amb*exp(-t_cool_rel/tau_off);
%   (paréntesis sin cerrar y exp() multiplicando solo a T_amb). Acá está
%   corregido.

    modelo_cool = @(tau, t) T_amb + (T0_cool - T_amb)*exp(-t/abs(tau));

    error_cool = @(tau) sum((T_cool - modelo_cool(tau, t_cool_rel)).^2);

    tau_off = abs(fminsearch(error_cool, tau_off0));

    T_pred_cool = T_amb + (T0_cool - T_amb)*exp(-t_cool_rel/tau_off);
end
