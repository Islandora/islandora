<?php

namespace Drupal\Tests\islandora\Functional;

use function GuzzleHttp\json_decode;

/**
 * Tests Jsonld Alter Reaction.
 *
 * @package Drupal\Tests\islandora\Functional
 * @group islandora
 */
class JsonldTypeAlterReactionTest extends JsonldSelfReferenceReactionTest {

  /**
   * @covers \Drupal\islandora\Plugin\ContextReaction\JsonldTypeAlterReaction
   */
  public function testMappingReaction() {
    $account = $this->drupalCreateUser([
      'bypass node access',
      'administer contexts',
      'administer node fields',
    ]);
    $this->drupalLogin($account);
    $this->drupalGet('admin/structure/types/manage/test_type/fields/add-field');

    // Add the typed predicate we will select in the reaction config.
    // Taken from FieldUiTestTrait->fieldUIAddNewField.
    if (version_compare(\Drupal::VERSION, '10.3.x-dev', 'lt')) {
      $this->getSession()->getPage()->selectFieldOption('new_storage_type', 'plain_text');
      // For Drupal 10.2, we first need to submit the form with the elements
      // displayed on initial page load. The form is using AJAX to send a
      // second element after we selected the radio button above
      // we can instead get the second element by submitting the form
      // and having it throw an error since the required field is missing.
      // @todo refactor this as a functional javascript test.
      $this->submitForm([
        'new_storage_type' => 'plain_text',
        'label' => 'Typed Predicate',
        'field_name' => 'type_predicate',
      ], 'Continue');

      // Now we can proceed, selecting the plain text (i.e. string)
      // for the second element now that the element is displayed after
      // the initial form submission.
      $this->getSession()->getPage()->selectFieldOption('group_field_options_wrapper', 'string');
      $this->submitForm([
        'new_storage_type' => 'plain_text',
        'label' => 'Typed Predicate',
        'field_name' => 'type_predicate',
        'group_field_options_wrapper' => 'string',
      ], 'Continue');
    }
    else {
      $this->getSession()->getPage()->selectFieldOption('new_storage_type', 'plain_text');
      // For Drupal 10.3 the label is not visible at first.
      // @todo refactor this as a functional javascript test.
      $this->submitForm([
        'new_storage_type' => 'plain_text',
      ], 'Continue');

      // Now we can proceed, entering a label and selecting Text (plain)
      // for the second element now that the elements are displayed after
      // the initial form submission.
      $this->getSession()->getPage()->selectFieldOption('group_field_options_wrapper', 'string');
      $this->submitForm([
        'label' => 'Typed Predicate',
        'field_name' => 'type_predicate',
        'group_field_options_wrapper' => 'string',
      ], 'Continue');
    }
    $this->submitForm([], 'Save settings');
    $this->assertSession()->responseContains('field_type_predicate');

    // Add the test node.
    $this->postNodeAddForm('test_type', [
      'title[0][value]' => 'Test Node',
      'field_type_predicate[0][value]' => 'schema:Organization',
    ], 'Save');
    $this->assertSession()->pageTextContains("Test Node");
    $url = $this->getUrl();

    // Make sure the node exists.
    $this->drupalGet($url);
    $this->assertSession()->statusCodeEquals(200);

    $contents = $this->drupalGet($url . '?_format=jsonld');
    $this->assertSession()->statusCodeEquals(200);
    $json = json_decode($contents, TRUE);
    $this->assertArrayHasKey('@type',
      $json['@graph'][0], 'Missing @type');
    $this->assertEquals(
      'http://schema.org/Thing',
      $json['@graph'][0]['@type'][0],
      'Missing @type value of http://schema.org/Thing'
    );

    // Add the test context.
    $context_name = 'test';
    $reaction_id = 'alter_jsonld_type';

    $this->createContext('Test', $context_name);
    $this->drupalGet("admin/structure/context/$context_name/reaction/add/$reaction_id");
    $this->assertSession()->statusCodeEquals(200);

    $this->drupalGet("admin/structure/context/$context_name");
    $this->getSession()->getPage()
      ->fillField("Field containing RDF type information", "field_type_predicate");
    $this->getSession()->getPage()->pressButton("Save and continue");
    $this->assertSession()
      ->pageTextContains("The context $context_name has been saved");

    $this->addCondition('test', 'islandora_entity_bundle');
    $this->getSession()->getPage()->checkField("edit-conditions-islandora-entity-bundle-bundles-test-type");
    $this->getSession()->getPage()->findById("edit-conditions-islandora-entity-bundle-context-mapping-node")->selectOption("@node.node_route_context:node");
    $this->getSession()->getPage()->pressButton('Save and continue');

    // The first time a Context is saved, you need to clear the cache.
    // Subsequent changes to the context don't need a cache rebuild, though.
    drupal_flush_all_caches();

    // Check for the new @type from the field_type_predicate value.
    $new_contents = $this->drupalGet($url . '?_format=jsonld');
    $json = json_decode($new_contents, TRUE);
    $this->assertTrue(
      in_array('http://schema.org/Organization', $json['@graph'][0]['@type']),
      'Missing altered @type value of http://schema.org/Organization'
    );
  }

}
