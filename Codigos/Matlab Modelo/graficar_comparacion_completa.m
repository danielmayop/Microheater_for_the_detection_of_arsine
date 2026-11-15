function graficar_comparacion_completa(resultados, ax)
% GRAFICAR_COMPARACION_COMPLETA Superpone el experimento completo
% (medido, y el modelo ajustado) de varios duty cycles en un mismo eje.
% El tiempo se referencia a t_on de cada archivo, para que el instante
% de encendido coincida en t=0 en todas las curvas.
%
%   GRAFICAR_COMPARACION_COMPLETA(resultados) abre una figura nueva.
%   GRAFICAR_COMPARACION_COMPLETA(resultados, ax) dibuja sobre el eje 'ax'.
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
        t_rel = r.t - r.t_on;
        plot(ax, t_rel, r.T, '-', 'Color', colores(i,:), 'LineWidth', 1.2);
        plot(ax, t_rel, r.T_pred_total, '--', 'Color', colores(i,:), ...
             'LineWidth', 1, 'HandleVisibility', 'off');
        leyendas{i} = sprintf('%d%% duty', r.duty);
    end
    hold(ax, 'off');

    grid(ax, 'on');
    xlabel(ax, 'Tiempo desde encendido [s]');
    ylabel(ax, 'Temperatura [°C]');
    title(ax, 'Comparacion del experimento completo por duty cycle');
    legend(ax, leyendas, 'Location', 'southeast');
end
