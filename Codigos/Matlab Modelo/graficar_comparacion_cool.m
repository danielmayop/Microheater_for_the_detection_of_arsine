function graficar_comparacion_cool(resultados, ax)
% GRAFICAR_COMPARACION_COOL Superpone las curvas de enfriamiento (medidas,
% y el modelo ajustado como referencia) de varios duty cycles en un mismo
% eje, usando tiempo relativo al apagado (t_off) para que todas arranquen
% en t=0 y sean comparables.
%
%   GRAFICAR_COMPARACION_COOL(resultados) abre una figura nueva.
%   GRAFICAR_COMPARACION_COOL(resultados, ax) dibuja sobre el eje 'ax'.
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
        plot(ax, r.t_cool_rel, r.T_cool, '-', 'Color', colores(i,:), 'LineWidth', 1.2);
        plot(ax, r.t_cool_rel, r.T_pred_cool, '--', 'Color', colores(i,:), ...
             'LineWidth', 1, 'HandleVisibility', 'off');
        leyendas{i} = sprintf('%d%% duty', r.duty);
    end
    hold(ax, 'off');

    grid(ax, 'on');
    xlabel(ax, 'Tiempo desde apagado [s]');
    ylabel(ax, 'Temperatura [°C]');
    title(ax, 'Comparacion de enfriamiento por duty cycle');
    legend(ax, leyendas, 'Location', 'northeast');
end
