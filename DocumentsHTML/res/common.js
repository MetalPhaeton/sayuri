$(function() {
    // 全て読み込み後イベント。
    $(window).on("load", function() {
        // 画像の幅を調整する。
        $("img").each(function(i) {
            if ($(this).width() > $("section.main_contents").width()) {
                $(this).width($("section.main_contents").width());
            }
        });

        // .figureの幅を画像の幅にする。
        $("div.figure").each(function(i) {
            $(this).width($("img", this).width());
        });
    });
});

