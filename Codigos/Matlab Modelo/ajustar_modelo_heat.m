function [K_on, tau_on, T_pred_heat] = ajustar_modelo_heat(t_heat_rel, T_heat, T_amb, K0, tau0)
% AJUSTAR_MODELO_HEAT Ajusta el modelo de primer orden de calentamiento
% por mínimos cuadrados no lineales (fminsearch).
%
%   Modelo: T(t) = T_amb + K*(1-exp(-t/tau))
%
%   Entradas:
%       t_heat_rel - tiempo relativo del tramo de calentamiento
%       T_heat     - temperatura medida del tramo de calentamiento
%       T_amb      - temperatura ambiente
%       K0, tau0   - valores iniciales para el ajuste
%
%   Salidas:
%       K_on, tau_on - parámetros ajustados
%       T_pred_heat  - temperatura predicha por el modelo ajustado

    modelo_heat = @(p, t) T_amb + p(1)*(1 - exp(-t/p(2)));
    p0 = [K0 tau0];

    error_heat = @(p) sum((T_heat - modelo_heat(p, t_heat_rel)).^2);

    % fminsearch no tiene restricciones; forzamos K>0, tau>0 con abs().
    funcion_objetivo_heat = @(p) error_heat([abs(p(1)) abs(p(2))]);

    p_heat = fminsearch(funcion_objetivo_heat, p0);

    K_on = abs(p_heat(1));
    tau_on = abs(p_heat(2));

    T_pred_heat = T_amb + K_on*(1 - exp(-t_heat_rel/tau_on));
end
