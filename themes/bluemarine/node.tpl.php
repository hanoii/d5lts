  <div class="node<?php if ($sticky) { print " sticky"; } ?>">
    <?php if ($picture) {
      print $picture;
    }?>
    <h2 class="title"><a href="<?php print $node_url?>"><?php print $title?></a></h2>
    <span class="submitted"><?php print $submitted?></span>
    <span class="taxonomy"><?php print $terms?></span>
    <div class="content"><?php print $content?></div>
    <div class="links">&raquo; <?php print $links?></div>
  </div>