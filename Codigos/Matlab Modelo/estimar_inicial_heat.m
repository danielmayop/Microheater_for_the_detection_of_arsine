function [K0, tau0, T_final] = estimar_inicial_heat(t_heat_rel, T_heat, T_amb)
% ESTIMAR_INICIAL_HEAT Estima valores iniciales K0 y tau0 para el ajuste
% del modelo de primer orden de calentamiento.
%
%   T(t) = T_amb + K*(1-exp(-t/tau))
%
%   Entradas:
%       t_heat_rel - tiempo relativo del tramo de calentamiento (empieza en 0)
%       T_heat     - temperatura del tramo de calentamiento
%       T_amb      - temperatura ambiente
%
%   Salidas:
%       K0      - ganancia inicial estimada
%       tau0    - constante de tiempo inicial estimada [s]
%       T_final - temperatura final de calentamiento (promedio de últimos puntos)

    % Usamos los ultimos 50 puntos para reducir el efecto del ruido.
    N = min(50, length(T_heat));
    T_final = mean(T_heat(end-N+1:end));

    K0 = T_final - T_amb; % para una entrada normalizada u=1

    % tau: instante donde se alcanza el 63.2% de la ganancia final
    T63 = T_amb + 0.632*K0;
    [~, indice_tau] = min(abs(T_heat - T63));
    tau0 = t_heat_rel(indice_tau);
end
