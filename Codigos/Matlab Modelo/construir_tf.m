function [G_on, G_off] = construir_tf(K_on, tau_on, tau_off)
% CONSTRUIR_TF Construye las funciones de transferencia de calentamiento
% y enfriamiento a partir de los parámetros identificados.
%
%   G_on(s)  = K_on / (tau_on*s + 1)
%   G_off(s) = 1 / (tau_off*s + 1)   (dinámica de enfriamiento, sin entrada
%                                      de potencia: dT/dt = -(T-T_amb)/tau_off)

    s = tf('s');
    G_on = K_on/(tau_on*s + 1);
    G_off = 1/(tau_off*s + 1);
end
