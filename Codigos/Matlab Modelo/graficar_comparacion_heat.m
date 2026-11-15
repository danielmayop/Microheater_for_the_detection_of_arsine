function graficar_comparacion_heat(resultados, ax)
% GRAFICAR_COMPARACION_HEAT Superpone las curvas de calentamiento
% (medidas, y el modelo ajustado como referencia) de varios duty cycles
% en un mismo eje, usando tiempo relativo al encendido (t_on) para que
% todas arranquen en t=0 y sean comparables.
%
%   GRAFICAR_COMPARACION_HEAT(resultados) abre una figura nueva.
%   GRAFICAR_COMPARACION_HEAT(resultados, ax) dibuja sobre el eje 'ax'.
%
%   Entrada:
%       resultados - cell array de structs devueltos por identificar_planta

    if nargin < 2 || isempty(ax)
        figure;
        ax = axes;
    end

    n = length(resultados);
    colores = lines(n);
    leyendas = cell(1, n);

    hold(ax, 'on');
    for i = 1:n
        r = resultados{i};
        plot(ax, r.t_heat_rel, r.T_heat, '-', 'Color', colores(i,:), 'LineWidth', 1.2);
        plot(ax, r.t_heat_rel, r.T_pred_heat, '--', 'Color', colores(i,:), ...
             'LineWidth', 1, 'HandleVisibility', 'off');
        leyendas{i} = sprintf('%d%% duty', r.duty);
    end
    hold(ax, 'off');

    grid(ax, 'on');
    xlabel(ax, 'Tiempo desde encendido [s]');
    ylabel(ax, 'Temperatura [°C]');
    title(ax, 'Comparacion de calentamiento por duty cycle');
    legend(ax, leyendas, 'Location', 'southeast');
end
